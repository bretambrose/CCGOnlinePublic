/**********************************************************************************************************************

	ODBCStatement.cpp
		A component defining an ODBC implementation of the abstract database statement interface.

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

#include "Database/DatabaseTypes.h"
#include <sqlext.h>
#include <sqlucode.h>
#include "Database/Interfaces/DatabaseVariableSetInterface.h"
#include "Database/Interfaces/DatabaseVariableInterface.h"

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
			return SQL_C_BIT;

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
			return SQL_CHAR;

		case DVVT_WSTRING:
			return SQL_WCHAR;

		case DVVT_FLOAT:
			return SQL_FLOAT;

		case DVVT_DOUBLE:
			return SQL_DOUBLE;

		case DVVT_BOOLEAN:
			return SQL_BIT;

		default:
			FATAL_ASSERT( false ); 
			return SQL_C_SLONG;
	}
}

enum ODBCStatementStateType
{
	ODBCCST_UNINITIALIZED,
	ODBCCST_INITIALIZED,
	ODBCCST_BOUND_INPUT,
	ODBCCST_READY,
	ODBCCST_PROCESS_RESULTS,
	ODBCCST_END_TRANSACTION,

	ODBCCST_SHUTDOWN,
	ODBCEST_FATAL_ERROR
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
	ODBCSOT_COMMIT_ROLLBACK_STATEMENT,
	ODBCSOT_FETCH_RESULTS
};

CODBCStatement::CODBCStatement( DBStatementIDType id, SQLHENV environment_handle, SQLHDBC connection_handle, SQLHSTMT statement_handle ) :
	BASECLASS( environment_handle, connection_handle, statement_handle ),
	ID( id ),
	State( ODBCCST_UNINITIALIZED ),
	ErrorState( DBEST_SUCCESS ),
	StatementText( L"" ),
	RowStatuses( nullptr ),
	RowsFetched( 0 ),
	ProcessResults( true )
{
}

CODBCStatement::~CODBCStatement()
{
	Shutdown();
}

void CODBCStatement::Initialize( const std::wstring &statement_text )
{
	FATAL_ASSERT( State == ODBCCST_UNINITIALIZED );

	StatementText = statement_text;

	State = ODBCCST_INITIALIZED;
}

void CODBCStatement::Shutdown( void )
{
	if ( State != ODBCCST_UNINITIALIZED && State != ODBCCST_SHUTDOWN )
	{
		if ( RowStatuses != nullptr )
		{
			delete []RowStatuses;
			RowStatuses = nullptr;
		}

		SQLFreeHandle( SQL_HANDLE_STMT, StatementHandle );

		Invalidate_Handles();
	}

	State = ODBCCST_SHUTDOWN;
}

void CODBCStatement::Bind_Input( IDatabaseVariableSet *param_set, uint32 param_set_size )
{
	FATAL_ASSERT( State != ODBCCST_SHUTDOWN && State != ODBCEST_FATAL_ERROR );

	if ( State != ODBCCST_INITIALIZED )
	{
		return;
	}

	std::vector< IDatabaseVariable * > parameters;
	param_set->Get_Variables( parameters );
	if ( parameters.size() > 0 )
	{
		SQLRETURN error_code = SQLSetStmtAttr( StatementHandle, SQL_ATTR_PARAM_BIND_TYPE, (SQLPOINTER) param_set_size, 0 );
		Update_Error_State( ODBCSOT_SET_PARAM_SET_ROW_SIZE, error_code );
		if ( !Was_Last_ODBC_Operation_Successful() )
		{
			State = ODBCEST_FATAL_ERROR;
			return;
		}

		for ( uint32 i = 0; i < parameters.size(); ++i )
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
			Update_Error_State( ODBCSOT_BIND_PARAMETER, error_code );
			if ( !Was_Last_ODBC_Operation_Successful() )
			{
				State = ODBCEST_FATAL_ERROR;
				return;
			}
		}
	}

	State = ODBCCST_BOUND_INPUT;
}

void CODBCStatement::Bind_Output( IDatabaseVariableSet *result_set, uint32 result_set_size, uint32 result_set_count )
{
	if ( State != ODBCCST_BOUND_INPUT )
	{
		return;
	}

	std::vector< IDatabaseVariable * > result_columns;
	result_set->Get_Variables( result_columns );
	if ( result_columns.size() > 0 )
	{
		ProcessResults = true;

		SQLRETURN error_code = SQLSetStmtAttr( StatementHandle, SQL_ATTR_ROW_BIND_TYPE, (SQLPOINTER) result_set_size, 0 );
		Update_Error_State( ODBCSOT_SET_RESULT_SET_ROW_SIZE, error_code );
		if ( !Was_Last_ODBC_Operation_Successful() )
		{
			State = ODBCEST_FATAL_ERROR;
			return;
		}

		error_code = SQLSetStmtAttr( StatementHandle, SQL_ATTR_ROW_ARRAY_SIZE, (SQLPOINTER) result_set_count, 0 );
		Update_Error_State( ODBCSOT_SET_RESULT_SET_ROW_COUNT, error_code );
		if ( !Was_Last_ODBC_Operation_Successful() )
		{
			State = ODBCEST_FATAL_ERROR;
			return;
		}

		FATAL_ASSERT( result_set_count > 0 );
		RowStatuses = new SQLSMALLINT[ result_set_count ];
		error_code = SQLSetStmtAttr( StatementHandle, SQL_ATTR_ROW_STATUS_PTR, RowStatuses, 0 );
		Update_Error_State( ODBCSOT_SET_ROW_STATUS_ARRAY, error_code );
		if ( !Was_Last_ODBC_Operation_Successful() )
		{
			State = ODBCEST_FATAL_ERROR;
			return;
		}

		error_code = SQLSetStmtAttr( StatementHandle, SQL_ATTR_ROWS_FETCHED_PTR, (SQLPOINTER) &RowsFetched, 0 );
		Update_Error_State( ODBCSOT_SET_ROWS_FETCHED_PTR, error_code );
		if ( !Was_Last_ODBC_Operation_Successful() )
		{
			State = ODBCEST_FATAL_ERROR;
			return;
		}

		for ( uint32 i = 0; i < result_columns.size(); ++i )
		{
			IDatabaseVariable *column = result_columns[ i ];
			SQLSMALLINT c_value_type = Get_ODBC_C_Value_Type( column->Get_Value_Type() );
			SQLPOINTER value_address = reinterpret_cast< SQLPOINTER >( column->Get_Value_Address() );
			SQLINTEGER value_buffer_size = column->Get_Value_Buffer_Size();
			SQLLEN *value_indicator = reinterpret_cast< SQLLEN * >( column->Get_Auxiliary_Address() );
			SQLUSMALLINT column_index = static_cast< SQLUSMALLINT >( i + 1 );

			error_code = SQLBindCol( StatementHandle, column_index, c_value_type, value_address, value_buffer_size, value_indicator );
			Update_Error_State( ODBCSOT_BIND_COLUMN, error_code );
			if ( !Was_Last_ODBC_Operation_Successful() )
			{
				State = ODBCEST_FATAL_ERROR;
				return;
			}
		}
	}
	else
	{
		ProcessResults = false;
	}

	State = ODBCCST_READY;
}

void CODBCStatement::Execute( uint32 batch_size )
{
	SQLRETURN error_code = SQLSetStmtAttr( StatementHandle, SQL_ATTR_PARAMSET_SIZE, (SQLPOINTER) batch_size, 0 );
	Update_Error_State( ODBCSOT_SET_PARAM_SET_COUNT, error_code );
	if ( !Was_Last_ODBC_Operation_Successful() )
	{
		State = ODBCEST_FATAL_ERROR;
		return;
	}

	error_code = SQLExecDirect( StatementHandle, (SQLWCHAR *)StatementText.c_str(), SQL_NTS );
	Update_Error_State( ODBCSOT_EXECUTE_STATEMENT, error_code );
	if ( !Was_Last_ODBC_Operation_Successful() )
	{
		State = ODBCEST_FATAL_ERROR;
		return;
	}

	State = ODBCCST_PROCESS_RESULTS;
}

EFetchResultsStatusType CODBCStatement::Fetch_Results( int64 &rows_fetched )
{
	if ( !ProcessResults )
	{
		State = ODBCCST_END_TRANSACTION;
	}

	if ( State == ODBCCST_END_TRANSACTION )
	{
		return FRST_FINISHED_ALL;
	}

	if ( State == ODBCEST_FATAL_ERROR )
	{
		return FRST_ERROR;
	}

	FATAL_ASSERT( State == ODBCCST_PROCESS_RESULTS );

	SQLRETURN error_code = SQLFetchScroll( StatementHandle, SQL_FETCH_NEXT, 0 );
	rows_fetched = RowsFetched;

	if ( error_code == SQL_NO_DATA )
	{
		SQLRETURN ec2 = SQLMoreResults( StatementHandle );
		if ( ec2 == SQL_NO_DATA_FOUND )
		{
			State = ODBCCST_END_TRANSACTION;
			return FRST_FINISHED_ALL;
		}
		else if ( ec2 != SQL_SUCCESS )
		{
			Update_Error_State( ODBCSOT_FETCH_RESULTS, error_code );
			State = ODBCEST_FATAL_ERROR;
			return FRST_ERROR;
		}
		else
		{
			return FRST_FINISHED_SET;
		}
	}
	else if ( error_code != SQL_SUCCESS )
	{
		Update_Error_State( ODBCSOT_FETCH_RESULTS, error_code );
		State = ODBCEST_FATAL_ERROR;
		return FRST_ERROR;
	}

	return FRST_ONGOING;
}

void CODBCStatement::End_Transaction( bool commit )
{
	FATAL_ASSERT( State == ODBCCST_END_TRANSACTION || ( !commit && State == ODBCEST_FATAL_ERROR ) );	

	SQLRETURN error_code = SQLEndTran( SQL_HANDLE_DBC, ConnectionHandle, commit ? SQL_COMMIT : SQL_ROLLBACK );
	Update_Error_State( ODBCSOT_COMMIT_ROLLBACK_STATEMENT, error_code );
	if ( !Was_Last_ODBC_Operation_Successful() )
	{
		State = ODBCEST_FATAL_ERROR;
		return;
	}

	State = ODBCCST_READY;
}

bool CODBCStatement::Is_Ready_For_Use( void ) const
{
	return State == ODBCCST_READY && ErrorState == DBEST_SUCCESS;
}

bool CODBCStatement::Was_Last_ODBC_Operation_Successful( void ) const
{
	return Was_Database_Operation_Successful( ErrorState );
}

void CODBCStatement::Update_Error_State( ODBCStatementOperationType operation_type, SQLRETURN error_code )
{
	if ( error_code == SQL_SUCCESS )
	{
		Clear_Error_List();
		ErrorState = DBEST_SUCCESS;
		return;
	}

	BASECLASS::Rebuild_Error_List();

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
		case ODBCSOT_EXECUTE_STATEMENT:
		case ODBCSOT_COMMIT_ROLLBACK_STATEMENT:
		case ODBCSOT_FETCH_RESULTS:
			ErrorState = DBEST_FATAL_ERROR;
			 
		default:
			FATAL_ASSERT( false );
			break;
	}
}

