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

#include <IPDatabase/ODBCImplementation/ODBCConnection.h>

#include <IPCore/Debug/DebugAssert.h>
#include <IPCore/Memory/Stl/StringStream.h>
#include <IPCore/Utils/StringUtils.h>
#include <IPDatabase/DatabaseTypes.h>
#include <IPDatabase/Interfaces/DatabaseTaskInterface.h>
#include <IPDatabase/Interfaces/DatabaseVariableInterface.h>
#include <IPDatabase/Interfaces/DatabaseVariableSetInterface.h>
#include <IPDatabase/ODBCImplementation/ODBCStatement.h>

#include <sqlext.h>
#include <sstream>

namespace IP
{
namespace Db
{

enum class ODBCConnectionStateType
{
	UNINITIALIZED,
	CONNECTED,

	SHUTDOWN,
	FATAL_ERROR
};

enum class ODBCConnectionOperationType
{
	CONNECT_TO_DB,
	ALLOCATE_STATEMENT_HANDLE,
	TOGGLE_AUTO_COMMIT,
	COMMIT
};

CODBCConnection::CODBCConnection( DBConnectionIDType id, SQLHENV environment_handle, SQLHDBC connection_handle, bool cache_statements ) :
	CODBCObjectBase( environment_handle, connection_handle, 0 ),
	ID( id ),
	State( ODBCConnectionStateType::UNINITIALIZED ),
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

void CODBCConnection::Initialize( const IP::String &connection_string )
{
	FATAL_ASSERT( State == ODBCConnectionStateType::UNINITIALIZED );

	SQLCHAR output_connection_buffer[ 1024 ] = { 0 };
	SQLSMALLINT output_size = 0;

	SQLRETURN error_code = SQLDriverConnect( ConnectionHandle, 
														  0, 
														  (SQLCHAR *)connection_string.c_str(), 
														  SQL_NTS, 
														  output_connection_buffer, 
														  sizeof( output_connection_buffer ), 
														  &output_size, 
														  SQL_DRIVER_COMPLETE );

	Update_Error_Status( ODBCConnectionOperationType::CONNECT_TO_DB, error_code );
	if ( !Was_Last_ODBC_Operation_Successful() )
	{
		State = ODBCConnectionStateType::FATAL_ERROR;
		return;
	}

	error_code = SQLSetConnectAttr( ConnectionHandle, SQL_ATTR_AUTOCOMMIT, (SQLPOINTER) SQL_AUTOCOMMIT_OFF, 0 );
	Update_Error_Status( ODBCConnectionOperationType::TOGGLE_AUTO_COMMIT, error_code );
	if ( !Was_Last_ODBC_Operation_Successful() )
	{
		State = ODBCConnectionStateType::FATAL_ERROR;
		return;
	}

	State = ODBCConnectionStateType::CONNECTED;
}

void CODBCConnection::Shutdown( void )
{
	if ( State != ODBCConnectionStateType::UNINITIALIZED && State != ODBCConnectionStateType::SHUTDOWN )
	{
		for ( auto iter = Statements.begin(); iter != Statements.end(); ++iter )
		{
			iter->second->Shutdown();
			IP::Delete( iter->second );
		}

		Statements.clear();
		CachedStatements.clear();

		SQLDisconnect( ConnectionHandle );
		SQLFreeHandle( SQL_HANDLE_DBC, ConnectionHandle );

		Invalidate_Handles();
	}

	State = ODBCConnectionStateType::SHUTDOWN;
}

IDatabaseStatement *CODBCConnection::Allocate_Statement( const IP::String &statement_text )
{
	FATAL_ASSERT( State != ODBCConnectionStateType::UNINITIALIZED );

	if ( State != ODBCConnectionStateType::CONNECTED )
	{
		return nullptr;
	}

	IP::String upper_statement_text;
	IP::StringUtils::To_Upper_Case( statement_text, upper_statement_text );
	auto iter = CachedStatements.find( upper_statement_text );
	if ( iter != CachedStatements.end() )
	{
		IDatabaseStatement *cached_statement = Get_Statement( iter->second );
		FATAL_ASSERT( cached_statement->Is_Ready_For_Use() );

		return cached_statement;
	}

	SQLHSTMT statement_handle = 0;
	SQLRETURN error_code = SQLAllocHandle( SQL_HANDLE_STMT, ConnectionHandle, &statement_handle );
	Update_Error_Status( ODBCConnectionOperationType::ALLOCATE_STATEMENT_HANDLE, error_code );
	if ( !Was_Last_ODBC_Operation_Successful() )
	{
		return nullptr;
	}

	DBStatementIDType new_id = NextStatementID;
	NextStatementID = static_cast< DBStatementIDType >( NextStatementID + 1 );
	IDatabaseStatement *new_statement = IP::New< CODBCStatement >( MEMORY_TAG, new_id, this, EnvironmentHandle, ConnectionHandle, statement_handle );
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
		IP::Delete( statement );

		Statements.erase( iter );
	}
	else
	{
		if ( !statement->Is_Ready_For_Use() )
		{
			DBStatementIDType statement_id = statement->Get_ID();

			statement->Shutdown();

			const IP::String &upper_statement_text = statement->Get_Statement_Text();
			auto iter = CachedStatements.find( upper_statement_text );
			if ( iter != CachedStatements.end() )
			{
				CachedStatements.erase( iter );
				Statements.erase( statement_id );
			}

			IP::Delete( statement );
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
		case ODBCConnectionOperationType::CONNECT_TO_DB:
			Set_Error_State_Base( ( error_code == SQL_SUCCESS_WITH_INFO ) ? DBEST_WARNING : DBEST_FATAL_ERROR );
			break;

		case ODBCConnectionOperationType::TOGGLE_AUTO_COMMIT:
			Set_Error_State_Base( DBEST_FATAL_ERROR );
			break;

		case ODBCConnectionOperationType::ALLOCATE_STATEMENT_HANDLE:
			Set_Error_State_Base( DBEST_RECOVERABLE_ERROR );
			break;

		case ODBCConnectionOperationType::COMMIT:
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

void CODBCConnection::Construct_Function_Procedure_Call_Statement_Text( IDatabaseTask *task, IDatabaseVariableSet *input_parameters, IP::String &statement_text ) const
{
	IP::OStringStream statement_stream;

	statement_stream << "{";
	if ( task->Get_Task_Type() == DTT_FUNCTION_CALL )
	{
		statement_stream << "? = ";
	}

	statement_stream << "call " << task->Get_Database_Object_Name() << "(";

	IP::Vector< IDatabaseVariable * > params;
	input_parameters->Get_Variables( params );

	uint32_t start_param = 0;
	if ( task->Get_Task_Type() == DTT_FUNCTION_CALL )
	{
		start_param = 1;
	}

	for ( uint32_t i = start_param; i < params.size(); ++i )
	{
		if ( i + 1 < params.size() )
		{
			statement_stream << "?,";
		}
		else
		{
			statement_stream << "?";
		}
	}

	statement_stream << ")}";

	statement_text = IP::String( statement_stream.rdbuf()->str() );
}

void CODBCConnection::Construct_Select_Statement_Text( IDatabaseTask *task, IP::String &statement_text ) const
{
	IP::OStringStream statement_stream;

	statement_stream << "SELECT ";

	IP::Vector< const char * > column_names;
	task->Build_Column_Name_List( column_names );

	for ( uint32_t i = 0; i < column_names.size(); ++i )
	{
		statement_stream << column_names[ i ];
		if ( i + 1 < column_names.size() )
		{
			statement_stream << ", ";
		}
	}

	statement_stream << " FROM " << task->Get_Database_Object_Name() << ";";

	statement_text = IP::String( statement_stream.rdbuf()->str() );
}

void CODBCConnection::Construct_Table_Valued_Function_Statement_Text( IDatabaseTask *task, IDatabaseVariableSet *input_parameters, IP::String &statement_text ) const
{
	IP::OStringStream statement_stream;

	statement_stream << "SELECT ";

	IP::Vector< const char * > column_names;
	task->Build_Column_Name_List( column_names );

	for ( uint32_t i = 0; i < column_names.size(); ++i )
	{
		statement_stream << column_names[ i ];
		if ( i + 1 < column_names.size() )
		{
			statement_stream << ", ";
		}
	}

	statement_stream << " FROM " << task->Get_Database_Object_Name() << "(";

	IP::Vector< IDatabaseVariable * > params;
	input_parameters->Get_Variables( params );

	for ( uint32_t i = 0; i < params.size(); ++i )
	{
		if ( i + 1 < params.size() )
		{
			statement_stream << "?, ";
		}
		else
		{
			statement_stream << "?";
		}
	}
		
	statement_stream << ");";

	statement_text = IP::String( statement_stream.rdbuf()->str() );
}

void CODBCConnection::Construct_Statement_Text( IDatabaseTask *task, IDatabaseVariableSet *input_parameters, IP::String &statement_text ) const
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

	IP::Vector< IDatabaseVariable * > input_params;
	input_parameters->Get_Variables( input_params );

	IP::Vector< IDatabaseVariable * > result_set;
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

			for ( uint32_t i = 1; i < input_params.size(); ++i )
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
			for ( uint32_t i = 0; i < input_params.size(); ++i )
			{
				EDatabaseVariableType variable_type = input_params[ i ]->Get_Parameter_Type();
				if ( variable_type != DVT_INPUT && variable_type != DVT_INPUT_OUTPUT )
				{
					return false;
				}
			}

			for ( uint32_t i = 0; i < result_set.size(); ++i )
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

			IP::Vector< const char * > column_names;
			task->Build_Column_Name_List( column_names );

			if ( column_names.size() != result_set.size() )
			{
				return false;
			}

			for ( uint32_t i = 0; i < result_set.size(); ++i )
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
			for ( uint32_t i = 0; i < input_params.size(); ++i )
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

			IP::Vector< const char * > column_names;
			task->Build_Column_Name_List( column_names );

			if ( column_names.size() != result_set.size() )
			{
				return false;
			}

			for ( uint32_t i = 0; i < result_set.size(); ++i )
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
	FATAL_ASSERT( State == ODBCConnectionStateType::CONNECTED );	

	SQLRETURN error_code = SQLEndTran( SQL_HANDLE_DBC, ConnectionHandle, commit ? SQL_COMMIT : SQL_ROLLBACK );
	Update_Error_Status( ODBCConnectionOperationType::COMMIT, error_code );
	if ( !Was_Last_ODBC_Operation_Successful() )
	{
		State = ODBCConnectionStateType::FATAL_ERROR;
		return;
	}
}

} // namespace Db
} // namespace IP