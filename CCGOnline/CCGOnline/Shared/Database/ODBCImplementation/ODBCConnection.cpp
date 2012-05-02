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

#include "ODBCStatement.h"
#include "Database/DatabaseTypes.h"
#include <sqlext.h>
#include "StringUtils.h"

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
	ODBCCOT_TOGGLE_AUTO_COMMIT
};

CODBCConnection::CODBCConnection( DBConnectionIDType id, SQLHENV environment_handle, SQLHDBC connection_handle, bool cache_statements ) :
	BASECLASS( environment_handle, connection_handle, 0 ),
	ID( id ),
	State( ODBCCST_UNINITIALIZED ),
	ErrorState( DBEST_SUCCESS ),
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

	Update_Error_State( ODBCCOT_CONNECT_TO_DB, error_code );
	if ( !Was_Last_ODBC_Operation_Successful() )
	{
		State = ODBCCST_FATAL_ERROR;
		return;
	}

	error_code = SQLSetConnectAttr( ConnectionHandle, SQL_ATTR_AUTOCOMMIT, (SQLPOINTER) SQL_AUTOCOMMIT_OFF, 0 );
	Update_Error_State( ODBCCOT_TOGGLE_AUTO_COMMIT, error_code );
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
	Update_Error_State( ODBCCOT_ALLOCATE_STATEMENT_HANDLE, error_code );
	if ( !Was_Last_ODBC_Operation_Successful() )
	{
		return nullptr;
	}

	DBStatementIDType new_id = NextStatementID;
	NextStatementID = static_cast< DBStatementIDType >( NextStatementID + 1 );
	IDatabaseStatement *new_statement = new CODBCStatement( new_id, EnvironmentHandle, ConnectionHandle, statement_handle );
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
			FATAL_ASSERT( iter != CachedStatements.end() );

			CachedStatements.erase( iter );
			Statements.erase( statement_id );

			delete statement;
		}
	}
}

bool CODBCConnection::Was_Last_ODBC_Operation_Successful( void ) const
{
	return Was_Database_Operation_Successful( ErrorState );
}

void CODBCConnection::Update_Error_State( ODBCConnectionOperationType operation_type, SQLRETURN error_code )
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
		case ODBCCOT_CONNECT_TO_DB:
			ErrorState = ( error_code == SQL_SUCCESS_WITH_INFO ) ? DBEST_WARNING : DBEST_FATAL_ERROR;
			break;

		case ODBCCOT_TOGGLE_AUTO_COMMIT:
			ErrorState = DBEST_FATAL_ERROR;
			break;

		case ODBCCOT_ALLOCATE_STATEMENT_HANDLE:
			ErrorState = DBEST_NON_FATAL_ERROR;
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
