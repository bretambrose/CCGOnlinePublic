/**********************************************************************************************************************

	ODBCConnection.cpp
		A component defining an ODBC implementation of the abstract database connection interface.

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

#include "ODBCConnection.h"

#include <sqlext.h>
#include <sstream>
#include "StringUtils.h"
#include "ODBCStatement.h"
#include "Database/DatabaseTypes.h"
#include "Database/Interfaces/DatabaseTaskInterface.h"
#include "Database/Interfaces/DatabaseVariableInterface.h"
#include "Database/Interfaces/DatabaseVariableSetInterface.h"

enum ODBCConnectionStateType
{
	ODBCCST_UNINITIALIZED,
	ODBCCST_CONNECTED,

	ODBCCST_SHUTDOWN,
	ODBCCST_FATAL_ERROR
};

enum ODBCConnectionOperationType
{
	ODBCCOT_CONNECT_TO_DB,
	ODBCCOT_ALLOCATE_STATEMENT_HANDLE,
	ODBCCOT_TOGGLE_AUTO_COMMIT,
	ODBCCOT_COMMIT
};

CODBCConnection::CODBCConnection( DBConnectionIDType id, SQLHENV environment_handle, SQLHDBC connection_handle, bool cache_statements ) :
	BASECLASS( environment_handle, connection_handle, 0 ),
	ID( id ),
	State( ODBCCST_UNINITIALIZED ),
	Statements(),
	CachedStatements(),
	NextStatementID( static_cast< DBStatementIDType >( 1 ) ),
	UseStatementCaching( cache_statements )
{
}

CODBCConnection::~CODBCConnection()
{
	Shutdown();
}

void CODBCConnection::Initialize( const std::wstring &connection_string )
{
	FATAL_ASSERT( State == ODBCCST_UNINITIALIZED );

	SQLWCHAR output_connection_buffer[ 1024 ] = { 0 };
	SQLSMALLINT output_size = 0;

	SQLRETURN error_code = SQLDriverConnect( ConnectionHandle, 
														  0, 
														  (SQLWCHAR *)connection_string.c_str(), 
														  SQL_NTS, 
														  output_connection_buffer, 
														  sizeof( output_connection_buffer ), 
														  &output_size, 
														  SQL_DRIVER_COMPLETE );

	Update_Error_Status( ODBCCOT_CONNECT_TO_DB, error_code );
	if ( !Was_Last_ODBC_Operation_Successful() )
	{
		State = ODBCCST_FATAL_ERROR;
		return;
	}

	error_code = SQLSetConnectAttr( ConnectionHandle, SQL_ATTR_AUTOCOMMIT, (SQLPOINTER) SQL_AUTOCOMMIT_OFF, 0 );
	Update_Error_Status( ODBCCOT_TOGGLE_AUTO_COMMIT, error_code );
	if ( !Was_Last_ODBC_Operation_Successful() )
	{
		State = ODBCCST_FATAL_ERROR;
		return;
	}

	State = ODBCCST_CONNECTED;
}

void CODBCConnection::Shutdown( void )
{
	if ( State != ODBCCST_UNINITIALIZED && State != ODBCCST_SHUTDOWN )
	{
		for ( auto iter = Statements.begin(); iter != Statements.end(); ++iter )
		{
			iter->second->Shutdown();
			delete iter->second;
		}

		Statements.clear();
		CachedStatements.clear();

		SQLDisconnect( ConnectionHandle );
		SQLFreeHandle( SQL_HANDLE_DBC, ConnectionHandle );

		Invalidate_Handles();
	}

	State = ODBCCST_SHUTDOWN;
}

IDatabaseStatement *CODBCConnection::Allocate_Statement( const std::wstring &statement_text )
{
	FATAL_ASSERT( State != ODBCCST_UNINITIALIZED );

	if ( State != ODBCCST_CONNECTED )
	{
		return nullptr;
	}

	std::wstring upper_statement_text;
	NStringUtils::To_Upper_Case( statement_text, upper_statement_text );
	auto iter = CachedStatements.find( upper_statement_text );
	if ( iter != CachedStatements.end() )
	{
		IDatabaseStatement *cached_statement = Get_Statement( iter->second );
		FATAL_ASSERT( cached_statement->Is_Ready_For_Use() );

		return cached_statement;
	}

	SQLHSTMT statement_handle = 0;
	SQLRETURN error_code = SQLAllocHandle( SQL_HANDLE_STMT, ConnectionHandle, &statement_handle );
	Update_Error_Status( ODBCCOT_ALLOCATE_STATEMENT_HANDLE, error_code );
	if ( !Was_Last_ODBC_Operation_Successful() )
	{
		return nullptr;
	}

	DBStatementIDType new_id = NextStatementID;
	NextStatementID = static_cast< DBStatementIDType >( NextStatementID + 1 );
	IDatabaseStatement *new_statement = new CODBCStatement( new_id, this, EnvironmentHandle, ConnectionHandle, statement_handle );
	new_statement->Initialize( upper_statement_text );

	if ( UseStatementCaching )
	{
		CachedStatements[ upper_statement_text ] = new_id;
	}

	Statements[ new_id ] = new_statement;

	return new_statement;
}

void CODBCConnection::Release_Statement( IDatabaseStatement *statement )
{
	if ( !UseStatementCaching )
	{
		FATAL_ASSERT( CachedStatements.size() == 0 );

		auto iter = Statements.find( statement->Get_ID() );
		FATAL_ASSERT( iter->second == statement );
		
		statement->Shutdown();
		delete statement;

		Statements.erase( iter );
	}
	else
	{
		if ( !statement->Is_Ready_For_Use() )
		{
			DBStatementIDType statement_id = statement->Get_ID();

			statement->Shutdown();

			const std::wstring &upper_statement_text = statement->Get_Statement_Text();
			auto iter = CachedStatements.find( upper_statement_text );
			if ( iter != CachedStatements.end() )
			{
				CachedStatements.erase( iter );
				Statements.erase( statement_id );
			}

			delete statement;
		}
	}
}

bool CODBCConnection::Was_Last_ODBC_Operation_Successful( void ) const
{
	return Was_Database_Operation_Successful( Get_Error_State_Base() );
}

void CODBCConnection::Update_Error_Status( ODBCConnectionOperationType operation_type, SQLRETURN error_code )
{
	if ( Refresh_Error_Status( error_code ) )
	{
		return;
	}

	switch ( operation_type )
	{
		case ODBCCOT_CONNECT_TO_DB:
			Set_Error_State_Base( ( error_code == SQL_SUCCESS_WITH_INFO ) ? DBEST_WARNING : DBEST_FATAL_ERROR );
			break;

		case ODBCCOT_TOGGLE_AUTO_COMMIT:
			Set_Error_State_Base( DBEST_FATAL_ERROR );
			break;

		case ODBCCOT_ALLOCATE_STATEMENT_HANDLE:
			Set_Error_State_Base( DBEST_RECOVERABLE_ERROR );
			break;

		case ODBCCOT_COMMIT:
			Set_Error_State_Base( DBEST_FATAL_ERROR );
			break;

		default:
			FATAL_ASSERT( false );
			break;
	}
}

IDatabaseStatement *CODBCConnection::Get_Statement( DBStatementIDType id ) const
{
	auto iter = Statements.find( id );
	if ( iter != Statements.end() )
	{
		return iter->second;
	}

	return nullptr;
}

void CODBCConnection::Construct_Function_Procedure_Call_Statement_Text( IDatabaseTask *task, IDatabaseVariableSet *input_parameters, std::wstring &statement_text ) const
{
	std::basic_ostringstream< wchar_t > statement_stream;

	statement_stream << L"{";
	if ( task->Get_Task_Type() == DTT_FUNCTION_CALL )
	{
		statement_stream << L"? = ";
	}

	statement_stream << L"call " << task->Get_Database_Object_Name() << L"(";

	std::vector< IDatabaseVariable * > params;
	input_parameters->Get_Variables( params );

	uint32 start_param = 0;
	if ( task->Get_Task_Type() == DTT_FUNCTION_CALL )
	{
		start_param = 1;
	}

	for ( uint32 i = start_param; i < params.size(); ++i )
	{
		if ( i + 1 < params.size() )
		{
			statement_stream << L"?,";
		}
		else
		{
			statement_stream << L"?";
		}
	}

	statement_stream << L")}";

	statement_text = std::wstring( statement_stream.rdbuf()->str() );
}

void CODBCConnection::Construct_Select_Statement_Text( IDatabaseTask *task, std::wstring &statement_text ) const
{
	std::basic_ostringstream< wchar_t > statement_stream;

	statement_stream << L"SELECT ";

	std::vector< const wchar_t * > column_names;
	task->Build_Column_Name_List( column_names );

	for ( uint32 i = 0; i < column_names.size(); ++i )
	{
		statement_stream << column_names[ i ];
		if ( i + 1 < column_names.size() )
		{
			statement_stream << L", ";
		}
	}

	statement_stream << L" FROM " << task->Get_Database_Object_Name() << L";";

	statement_text = std::wstring( statement_stream.rdbuf()->str() );
}

void CODBCConnection::Construct_Table_Valued_Function_Statement_Text( IDatabaseTask *task, IDatabaseVariableSet *input_parameters, std::wstring &statement_text ) const
{
	std::basic_ostringstream< wchar_t > statement_stream;

	statement_stream << L"SELECT ";

	std::vector< const wchar_t * > column_names;
	task->Build_Column_Name_List( column_names );

	for ( uint32 i = 0; i < column_names.size(); ++i )
	{
		statement_stream << column_names[ i ];
		if ( i + 1 < column_names.size() )
		{
			statement_stream << L", ";
		}
	}

	statement_stream << L" FROM " << task->Get_Database_Object_Name() << L"(";

	std::vector< IDatabaseVariable * > params;
	input_parameters->Get_Variables( params );

	for ( uint32 i = 0; i < params.size(); ++i )
	{
		if ( i + 1 < params.size() )
		{
			statement_stream << L"?, ";
		}
		else
		{
			statement_stream << L"?";
		}
	}
		
	statement_stream << L");";

	statement_text = std::wstring( statement_stream.rdbuf()->str() );
}

void CODBCConnection::Construct_Statement_Text( IDatabaseTask *task, IDatabaseVariableSet *input_parameters, std::wstring &statement_text ) const
{
	switch ( task->Get_Task_Type() )
	{
		case DTT_FUNCTION_CALL:
		case DTT_PROCEDURE_CALL:
			Construct_Function_Procedure_Call_Statement_Text( task, input_parameters, statement_text );
			break;

		case DTT_SELECT:
			Construct_Select_Statement_Text( task, statement_text );
			break;

		case DTT_TABLE_VALUED_FUNCTION_CALL:
			Construct_Table_Valued_Function_Statement_Text( task, input_parameters, statement_text );
			break;
	}
}

bool CODBCConnection::Validate_Input_Output_Signatures( IDatabaseTask *task, IDatabaseVariableSet *input_parameters, IDatabaseVariableSet *output_parameters ) const
{
	FATAL_ASSERT( task != nullptr && input_parameters != nullptr && output_parameters != nullptr );

	EDatabaseTaskType task_type = task->Get_Task_Type();

	std::vector< IDatabaseVariable * > input_params;
	input_parameters->Get_Variables( input_params );

	std::vector< IDatabaseVariable * > result_set;
	output_parameters->Get_Variables( result_set );

	switch ( task_type )
	{
		case DTT_FUNCTION_CALL:
		{
			if ( input_params.size() == 0 )
			{
				return false;
			}

			if ( input_params[ 0 ]->Get_Parameter_Type() != DVT_OUTPUT )
			{
				return false;
			}

			for ( uint32 i = 1; i < input_params.size(); ++i )
			{
				if ( input_params[ i ]->Get_Parameter_Type() != DVT_INPUT )	// legal to have functions with in/out params?
				{
					return false;
				}
			}

			if ( result_set.size() != 0 )
			{
				return false;
			}

			break;
		}

		case DTT_PROCEDURE_CALL:
		{
			for ( uint32 i = 0; i < input_params.size(); ++i )
			{
				EDatabaseVariableType variable_type = input_params[ i ]->Get_Parameter_Type();
				if ( variable_type != DVT_INPUT && variable_type != DVT_INPUT_OUTPUT )
				{
					return false;
				}
			}

			for ( uint32 i = 0; i < result_set.size(); ++i )
			{
				if ( result_set[ i ]->Get_Parameter_Type() != DVT_INPUT )
				{
					return false;
				}
			}

			break;
		}

		case DTT_SELECT:
		{
			if ( input_params.size() != 0 )
			{
				return false;
			}

			if ( result_set.size() == 0 )
			{
				return false;			// what's the point of selecting nothing?
			}

			std::vector< const wchar_t * > column_names;
			task->Build_Column_Name_List( column_names );

			if ( column_names.size() != result_set.size() )
			{
				return false;
			}

			for ( uint32 i = 0; i < result_set.size(); ++i )
			{
				if ( result_set[ i ]->Get_Parameter_Type() != DVT_INPUT )
				{
					return false;
				}
			}

			break;
		}

		case DTT_TABLE_VALUED_FUNCTION_CALL:
		{
			for ( uint32 i = 0; i < input_params.size(); ++i )
			{
				if ( input_params[ i ]->Get_Parameter_Type() != DVT_INPUT )
				{
					return false;
				}
			}

			if ( result_set.size() == 0 )
			{
				return false;			// what's the point of selecting nothing?
			}

			std::vector< const wchar_t * > column_names;
			task->Build_Column_Name_List( column_names );

			if ( column_names.size() != result_set.size() )
			{
				return false;
			}

			for ( uint32 i = 0; i < result_set.size(); ++i )
			{
				if ( result_set[ i ]->Get_Parameter_Type() != DVT_INPUT )
				{
					return false;
				}
			}

			break;
		}

		default:
			FATAL_ASSERT( false );
			break;
	}

	return true;
}

void CODBCConnection::End_Transaction( bool commit )
{
	FATAL_ASSERT( State == ODBCCST_CONNECTED );	

	SQLRETURN error_code = SQLEndTran( SQL_HANDLE_DBC, ConnectionHandle, commit ? SQL_COMMIT : SQL_ROLLBACK );
	Update_Error_Status( ODBCCOT_COMMIT, error_code );
	if ( !Was_Last_ODBC_Operation_Successful() )
	{
		State = ODBCCST_FATAL_ERROR;
		return;
	}
}