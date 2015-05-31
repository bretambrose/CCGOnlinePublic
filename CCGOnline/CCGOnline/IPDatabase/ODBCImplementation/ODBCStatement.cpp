/**********************************************************************************************************************

	(c) Copyright 2012, Bret Ambrose (mailto:bretambrose@gmail.com).

	This program is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program.  If not, see <http://www.gnu.org/licenses/>.

**********************************************************************************************************************/

#include "stdafx.h"

#include "ODBCStatement.h"

#include "IPDatabase/DatabaseTypes.h"
#include <sqlext.h>
#include <sqlucode.h>
#include "IPDatabase/Interfaces/DatabaseVariableSetInterface.h"
#include "IPDatabase/Interfaces/DatabaseVariableInterface.h"
#include "IPShared/Logging/LogInterface.h"

/*
Controls whether or not to aggressively try and determine which store proc call caused an error.  The conservative approach is
to retry the entire batch one by one, which is slow.  The aggressive approach is to use conditionally use the row number value (whose validity is conditional
and not particularly well-defined) and the current result set number to try and assert which call is the problem.  This is a heuristic approach
and may fail in the face of poorly-developed store procs.
*/
// #define CONSERVATIVE_ODBC_ERROR_HANDLING

SQLSMALLINT Get_ODBC_Variable_Type( EDatabaseVariableType variable_type )
{
	switch ( variable_type )
	{
		case DVT_INPUT:
			return SQL_PARAM_INPUT;

		case DVT_INPUT_OUTPUT:
			return SQL_PARAM_INPUT_OUTPUT;

		case DVT_OUTPUT:
			return SQL_PARAM_OUTPUT;

		default:
			FATAL_ASSERT( false );
			return SQL_PARAM_INPUT;
	}
}

SQLSMALLINT Get_ODBC_C_Value_Type( EDatabaseVariableValueType variable_value_type )
{
	switch ( variable_value_type )
	{
		case DVVT_INT32:
			return SQL_C_SLONG;

		case DVVT_UINT32:
			return SQL_C_ULONG;

		case DVVT_INT64:
			return SQL_C_SBIGINT;

		case DVVT_UINT64:
			return SQL_C_UBIGINT;

		case DVVT_STRING:
			return SQL_C_CHAR;

		case DVVT_WSTRING:
			return SQL_C_WCHAR;

		case DVVT_FLOAT:
			return SQL_C_FLOAT;

		case DVVT_DOUBLE:
			return SQL_C_DOUBLE;

		case DVVT_BOOLEAN:
			return SQL_C_BIT;		// or SQL_C_TINYINT?

		default:
			FATAL_ASSERT( false ); 
			return SQL_C_SLONG;
	}

}

SQLSMALLINT Get_ODBC_SQL_Value_Type( EDatabaseVariableValueType variable_value_type )
{
	switch ( variable_value_type )
	{
		case DVVT_INT32:
		case DVVT_UINT32:
			return SQL_INTEGER;

		case DVVT_INT64:
		case DVVT_UINT64:
			return SQL_BIGINT;

		case DVVT_STRING:
			return SQL_VARCHAR;

		case DVVT_WSTRING:
			return SQL_WVARCHAR;

		case DVVT_FLOAT:
			return SQL_FLOAT;

		case DVVT_DOUBLE:
			return SQL_DOUBLE;

		case DVVT_BOOLEAN:
			return SQL_BIT;		// or SQL_TINYINT?

		default:
			FATAL_ASSERT( false ); 
			return SQL_C_SLONG;
	}
}

enum ODBCStatementStateType
{
	ODBCSST_UNINITIALIZED,
	ODBCSST_INITIALIZED,
	ODBCSST_BOUND_INPUT,
	ODBCSST_READY,
	ODBCSST_PROCESS_RESULTS,

	ODBCSST_SHUTDOWN,
	ODBCSST_RECOVERABLE_ERROR,
	ODBCSST_FATAL_ERROR
};

enum ODBCStatementOperationType
{
	ODBCSOT_SET_PARAM_SET_ROW_SIZE,
	ODBCSOT_BIND_PARAMETER,
	ODBCSOT_SET_PARAM_SET_COUNT,

	ODBCSOT_SET_RESULT_SET_ROW_SIZE,
	ODBCSOT_SET_RESULT_SET_ROW_COUNT,
	ODBCSOT_SET_ROW_STATUS_ARRAY,
	ODBCSOT_SET_ROWS_FETCHED_PTR,
	ODBCSOT_BIND_COLUMN,

	ODBCSOT_EXECUTE_STATEMENT,
	ODBCSOT_CHECK_COLUMN_COUNT,
	ODBCSOT_FETCH_RESULT_ROWS,
	ODBCSOT_MOVE_TO_NEXT_RESULT_SET
};

CODBCStatement::CODBCStatement( DBStatementIDType id, IDatabaseConnection *connection, SQLHENV environment_handle, SQLHDBC connection_handle, SQLHSTMT statement_handle ) :
	BASECLASS( environment_handle, connection_handle, statement_handle ),
	ID( id ),
	Connection( connection ),
	State( ODBCSST_UNINITIALIZED ),
	StatementText( L"" ),
	ResultSetRowCount( 1 ),
	RowStatuses( nullptr ),
	RowsFetched( 0 ),
	ExpectedResultSetWidth( 0 ),
	CurrentResultSetWidth( -1 ),
	CurrentResultSet( -1 )
{
}

CODBCStatement::~CODBCStatement()
{
	Shutdown();
}

void CODBCStatement::Reflect_ODBC_Error_State_Into_Statement_State( void )
{
	DBErrorStateType error_state = Get_Error_State_Base();
	if ( error_state == DBEST_RECOVERABLE_ERROR )
	{
		State = ODBCSST_RECOVERABLE_ERROR;
	}
	else if ( error_state == DBEST_FATAL_ERROR )
	{
		State = ODBCSST_FATAL_ERROR;
	}
}

void CODBCStatement::Initialize( const std::wstring &statement_text )
{
	FATAL_ASSERT( State == ODBCSST_UNINITIALIZED );

	StatementText = statement_text;

	State = ODBCSST_INITIALIZED;
}

void CODBCStatement::Shutdown( void )
{
	if ( State != ODBCSST_UNINITIALIZED && State != ODBCSST_SHUTDOWN )
	{
		if ( RowStatuses != nullptr )
		{
			delete []RowStatuses;
			RowStatuses = nullptr;
		}

		SQLFreeHandle( SQL_HANDLE_STMT, StatementHandle );

		Invalidate_Handles();
	}

	Connection = nullptr;

	State = ODBCSST_SHUTDOWN;
}

void CODBCStatement::Bind_Input( IDatabaseVariableSet *param_set, uint32_t param_set_size )
{
	FATAL_ASSERT( State == ODBCSST_INITIALIZED );

	std::vector< IDatabaseVariable * > parameters;
	param_set->Get_Variables( parameters );
	if ( parameters.size() > 0 )
	{
		SQLRETURN error_code = SQLSetStmtAttr( StatementHandle, SQL_ATTR_PARAM_BIND_TYPE, (SQLPOINTER) param_set_size, 0 );
		Update_Error_Status( ODBCSOT_SET_PARAM_SET_ROW_SIZE, error_code );
		if ( !Was_Last_ODBC_Operation_Successful() )
		{
			Reflect_ODBC_Error_State_Into_Statement_State();
			return;
		}

		for ( uint32_t i = 0; i < parameters.size(); ++i )
		{
			IDatabaseVariable *parameter = parameters[ i ];

			SQLSMALLINT param_type = Get_ODBC_Variable_Type( parameter->Get_Parameter_Type() );
			SQLSMALLINT c_value_type = Get_ODBC_C_Value_Type( parameter->Get_Value_Type() );
			SQLSMALLINT sql_value_type = Get_ODBC_SQL_Value_Type( parameter->Get_Value_Type() );
			SQLUINTEGER value_size = parameter->Get_Value_Size();
			SQLSMALLINT decimals = static_cast< SQLSMALLINT >( parameter->Get_Decimals() );
			SQLPOINTER value_address = reinterpret_cast< SQLPOINTER >( parameter->Get_Value_Address() );
			SQLINTEGER value_buffer_size = parameter->Get_Value_Buffer_Size();
			SQLLEN *indicator_address = reinterpret_cast< SQLLEN * >( parameter->Get_Auxiliary_Address() );
			SQLUSMALLINT parameter_index = static_cast< SQLUSMALLINT >( i + 1 );

			error_code = SQLBindParameter( StatementHandle, parameter_index, param_type, c_value_type, sql_value_type, value_size, decimals, value_address, value_buffer_size, indicator_address );
			Update_Error_Status( ODBCSOT_BIND_PARAMETER, error_code );
			if ( !Was_Last_ODBC_Operation_Successful() )
			{
				Reflect_ODBC_Error_State_Into_Statement_State();
				return;
			}
		}
	}

	State = ODBCSST_BOUND_INPUT;
}

void CODBCStatement::Bind_Output( IDatabaseVariableSet *result_set, uint32_t result_set_size, uint32_t result_set_count )
{
	FATAL_ASSERT( result_set_count > 0 );
	ResultSetRowCount = result_set_count;

	FATAL_ASSERT( State == ODBCSST_BOUND_INPUT );

	std::vector< IDatabaseVariable * > result_columns;
	result_set->Get_Variables( result_columns );
	if ( result_columns.size() > 0 )
	{
		ExpectedResultSetWidth = static_cast< int32_t >( result_columns.size() );

		SQLRETURN error_code = SQLSetStmtAttr( StatementHandle, SQL_ATTR_ROW_BIND_TYPE, (SQLPOINTER) result_set_size, 0 );
		Update_Error_Status( ODBCSOT_SET_RESULT_SET_ROW_SIZE, error_code );
		if ( !Was_Last_ODBC_Operation_Successful() )
		{
			Reflect_ODBC_Error_State_Into_Statement_State();
			return;
		}

		FATAL_ASSERT( result_set_count > 0 );
		RowStatuses = new SQLSMALLINT[ result_set_count ];
		error_code = SQLSetStmtAttr( StatementHandle, SQL_ATTR_ROW_STATUS_PTR, RowStatuses, 0 );
		Update_Error_Status( ODBCSOT_SET_ROW_STATUS_ARRAY, error_code );
		if ( !Was_Last_ODBC_Operation_Successful() )
		{
			Reflect_ODBC_Error_State_Into_Statement_State();
			return;
		}

		error_code = SQLSetStmtAttr( StatementHandle, SQL_ATTR_ROWS_FETCHED_PTR, (SQLPOINTER) &RowsFetched, 0 );
		Update_Error_Status( ODBCSOT_SET_ROWS_FETCHED_PTR, error_code );
		if ( !Was_Last_ODBC_Operation_Successful() )
		{
			Reflect_ODBC_Error_State_Into_Statement_State();
			return;
		}

		for ( uint32_t i = 0; i < result_columns.size(); ++i )
		{
			IDatabaseVariable *column = result_columns[ i ];
			SQLSMALLINT c_value_type = Get_ODBC_C_Value_Type( column->Get_Value_Type() );
			SQLPOINTER value_address = reinterpret_cast< SQLPOINTER >( column->Get_Value_Address() );
			SQLINTEGER value_buffer_size = column->Get_Value_Buffer_Size();
			SQLLEN *value_indicator = reinterpret_cast< SQLLEN * >( column->Get_Auxiliary_Address() );
			SQLUSMALLINT column_index = static_cast< SQLUSMALLINT >( i + 1 );

			error_code = SQLBindCol( StatementHandle, column_index, c_value_type, value_address, value_buffer_size, value_indicator );
			Update_Error_Status( ODBCSOT_BIND_COLUMN, error_code );
			if ( !Was_Last_ODBC_Operation_Successful() )
			{
				Reflect_ODBC_Error_State_Into_Statement_State();
				return;
			}
		}
	}

	State = ODBCSST_READY;
}

void CODBCStatement::Execute( uint32_t batch_size )
{
	if ( State != ODBCSST_READY )
	{
		return;
	}

	// Good freaking lord I hate ODBC; this needs to be 1 at the time execute is called or it uses a server-side cursor that's
	// all kinds of fucked up.  We set it to the real value right before results processing.
	SQLRETURN error_code = SQLSetStmtAttr( StatementHandle, SQL_ATTR_ROW_ARRAY_SIZE, (SQLPOINTER) 1, 0 );
	Update_Error_Status( ODBCSOT_SET_RESULT_SET_ROW_COUNT, error_code );
	if ( !Was_Last_ODBC_Operation_Successful() )
	{
		Reflect_ODBC_Error_State_Into_Statement_State();
		return;
	}

	error_code = SQLSetStmtAttr( StatementHandle, SQL_ATTR_PARAMSET_SIZE, (SQLPOINTER) batch_size, 0 );
	Update_Error_Status( ODBCSOT_SET_PARAM_SET_COUNT, error_code );
	if ( !Was_Last_ODBC_Operation_Successful() )
	{
		Reflect_ODBC_Error_State_Into_Statement_State();
		return;
	}

	error_code = SQLExecDirect( StatementHandle, (SQLWCHAR *)StatementText.c_str(), SQL_NTS );
	Update_Error_Status( ODBCSOT_EXECUTE_STATEMENT, error_code );
	if ( !Was_Last_ODBC_Operation_Successful() )
	{
		Reflect_ODBC_Error_State_Into_Statement_State();
		return;
	}

	// Set result set row count to the real value in anticipation of result set processing
	error_code = SQLSetStmtAttr( StatementHandle, SQL_ATTR_ROW_ARRAY_SIZE, (SQLPOINTER) ResultSetRowCount, 0 );
	Update_Error_Status( ODBCSOT_SET_RESULT_SET_ROW_COUNT, error_code );
	if ( !Was_Last_ODBC_Operation_Successful() )
	{
		Reflect_ODBC_Error_State_Into_Statement_State();
		return;
	}

	CurrentResultSetWidth = -1;
	CurrentResultSet = 0;
	State = ODBCSST_PROCESS_RESULTS;
}

EFetchResultsStatusType CODBCStatement::Fetch_Results( int64_t &rows_fetched )
{
	rows_fetched = 0;

	if ( State == ODBCSST_READY )
	{
		return FRST_FINISHED_ALL;
	}

	if ( State == ODBCSST_FATAL_ERROR || State == ODBCSST_RECOVERABLE_ERROR )
	{
		return FRST_ERROR;
	}

	FATAL_ASSERT( State == ODBCSST_PROCESS_RESULTS );

	// If this is our first encounter with this result set, get its width; make sure it matches what we're expecting
	if ( CurrentResultSetWidth == -1 )
	{
		SQLSMALLINT column_count = 0;
		SQLRETURN ec = SQLNumResultCols( StatementHandle, &column_count );
		if ( ec != SQL_SUCCESS )
		{
			Update_Error_Status( ODBCSOT_CHECK_COLUMN_COUNT, ec );
			Reflect_ODBC_Error_State_Into_Statement_State();
			return FRST_ERROR;
		}

		CurrentResultSetWidth = column_count;

		if ( CurrentResultSetWidth != ExpectedResultSetWidth )
		{
			std::basic_ostringstream< wchar_t > message_stream;
			message_stream << L"\tGot a result set of width " << CurrentResultSetWidth << L" while expecting a result set of width " << ExpectedResultSetWidth;
			Push_User_Error( DBEST_RECOVERABLE_ERROR, message_stream.rdbuf()->str() );
			State = ODBCSST_RECOVERABLE_ERROR;
			return FRST_ERROR;
		}
	}

	// SQLMoreResults is the trigger that allows us to read final values from our in/out parameters, so we must call it even if there's no result set
	// On the other hand, calling FetchScroll when there's no results is bad, so skip if appropriate
	SQLRETURN error_code = SQL_NO_DATA;	
	if ( CurrentResultSetWidth > 0 && ExpectedResultSetWidth > 0 )
	{
		error_code = SQLFetchScroll( StatementHandle, SQL_FETCH_NEXT, 0 );
		rows_fetched = RowsFetched;
	}

	if ( error_code == SQL_NO_DATA )
	{
		CurrentResultSetWidth = -1;
		CurrentResultSet++;	
		SQLRETURN ec2 = SQLMoreResults( StatementHandle );
		if ( ec2 == SQL_NO_DATA_FOUND )
		{
			State = ODBCSST_READY;
			return FRST_FINISHED_ALL;
		}
		else if ( ec2 != SQL_SUCCESS )
		{
			Update_Error_Status( ODBCSOT_MOVE_TO_NEXT_RESULT_SET, error_code );
			Reflect_ODBC_Error_State_Into_Statement_State();
			return FRST_ERROR;
		}
		else
		{
			return FRST_FINISHED_SET;
		}
	}
	else if ( error_code != SQL_SUCCESS )
	{
		Update_Error_Status( ODBCSOT_FETCH_RESULT_ROWS, error_code );
		Reflect_ODBC_Error_State_Into_Statement_State();

		// experiment: try to empty out result set before returning
		while ( RowsFetched > 0 && error_code != SQL_NO_DATA )
		{
			error_code = SQLFetchScroll( StatementHandle, SQL_FETCH_NEXT, 0 );
		}

		return FRST_ERROR;
	}

	return FRST_ONGOING;
}

bool CODBCStatement::Needs_Binding( void ) const
{
	return State == ODBCSST_INITIALIZED;
}

bool CODBCStatement::Is_Ready_For_Use( void ) const
{
	return State == ODBCSST_READY && Get_Error_State_Base() == DBEST_SUCCESS;
}

bool CODBCStatement::Is_In_Error_State( void ) const
{
	return State == ODBCSST_FATAL_ERROR || State == ODBCSST_RECOVERABLE_ERROR;
}

bool CODBCStatement::Was_Last_ODBC_Operation_Successful( void ) const
{
	return Was_Database_Operation_Successful( Get_Error_State_Base() );
}

void CODBCStatement::Update_Error_Status( ODBCStatementOperationType operation_type, SQLRETURN error_code )
{
	if ( Refresh_Error_Status( error_code ) )
	{
		return;
	}

	switch ( operation_type )
	{
		case ODBCSOT_SET_PARAM_SET_ROW_SIZE:
		case ODBCSOT_BIND_PARAMETER:
		case ODBCSOT_SET_PARAM_SET_COUNT:
		case ODBCSOT_SET_RESULT_SET_ROW_SIZE:
		case ODBCSOT_SET_RESULT_SET_ROW_COUNT:
		case ODBCSOT_SET_ROW_STATUS_ARRAY:
		case ODBCSOT_SET_ROWS_FETCHED_PTR:
		case ODBCSOT_BIND_COLUMN:
		case ODBCSOT_CHECK_COLUMN_COUNT:
			Set_Error_State_Base( DBEST_FATAL_ERROR );
			break;

		case ODBCSOT_EXECUTE_STATEMENT:
			Set_Error_State_Base( DBEST_RECOVERABLE_ERROR );
			break;

		case ODBCSOT_MOVE_TO_NEXT_RESULT_SET:
		case ODBCSOT_FETCH_RESULT_ROWS:
			Set_Error_State_Base( DBEST_RECOVERABLE_ERROR );
#ifdef CONSERVATIVE_ODBC_ERROR_HANDLING
			Set_Bad_Row_Number_Base( -1 );
#else
			Set_Bad_Row_Number_Base( CurrentResultSet );
#endif // 
			break;
						 
		default:
			FATAL_ASSERT( false );
			break;
	}
}

void CODBCStatement::Log_Error_State( void ) const
{
	WLOG( LL_LOW, L"\tStatement Text: " << StatementText.c_str() );

	BASECLASS::Log_Error_State_Base();
}

void CODBCStatement::Return_To_Ready( void )
{
	if ( State == ODBCSST_READY )
	{	
		return;
	}

	FATAL_ASSERT( State == ODBCSST_RECOVERABLE_ERROR || State == ODBCSST_PROCESS_RESULTS );  

	Refresh_Error_Status( SQL_SUCCESS);

	State = ODBCSST_READY;
}
