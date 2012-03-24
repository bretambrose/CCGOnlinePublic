/**********************************************************************************************************************

	AuthServer.cpp
		Entry point for auth server application

	(c) Copyright 2011, Bret Ambrose (mailto:bretambrose@gmail.com).

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

#include "ServerShared.h"

#include <Windows.h>
#include <sql.h>
#include <sqlext.h>
#include <sqltypes.h>

#include "GeneratedCode\RegisterAuthServerEnums.h"
#include "Logging/LogInterface.h"

void SQL_Stuff( void )
{

	SQLHENV env_handle = 0;
	SQLRETURN error_code = SQLAllocHandle( SQL_HANDLE_ENV, SQL_NULL_HANDLE, &env_handle );
	assert( error_code == SQL_SUCCESS );

	error_code = SQLSetEnvAttr( env_handle, SQL_ATTR_ODBC_VERSION, (void*) SQL_OV_ODBC3, 0 );
	assert( error_code == SQL_SUCCESS );

	SQLHDBC dbc_handle = 0;
	error_code = SQLAllocHandle( SQL_HANDLE_DBC, env_handle, &dbc_handle );
	assert( error_code == SQL_SUCCESS );

	SQLWCHAR output_connection_buffer[ 1024 ] = { 0 };
	SQLSMALLINT output_size = 0;

	error_code = SQLDriverConnect( dbc_handle, 
											 0, 
											 (SQLWCHAR *)L"dsn=ODBCTestDS", 
											 SQL_NTS, 
											 output_connection_buffer, 
											 1024, 
											 &output_size, 
											 SQL_DRIVER_COMPLETE );
	if ( error_code == SQL_SUCCESS )
	{
		SQLDisconnect( dbc_handle );
	}
	else
	{
		SQLSMALLINT text_length = 0;
		SQLINTEGER sql_error_code = 0;
		SQLWCHAR error_buffer[ 1024 ] = { 0 };
		SQLWCHAR sql_state[ 6 ] = { 0 };

		error_code = SQLError( env_handle, dbc_handle, SQL_NULL_HSTMT, sql_state, &sql_error_code, error_buffer, 1024, &text_length );
		while ( error_code == SQL_SUCCESS )
		{
			error_code = SQLError( env_handle, dbc_handle, SQL_NULL_HSTMT, sql_state, &sql_error_code, error_buffer, 1024, &text_length );
		}
	}

	SQLFreeHandle( SQL_HANDLE_ENV, dbc_handle );
	SQLFreeHandle( SQL_HANDLE_ENV, env_handle );
}

void Sql_Stuff2( void )
{
	SQLHENV env_handle = 0;
	SQLRETURN error_code = SQLAllocHandle( SQL_HANDLE_ENV, SQL_NULL_HANDLE, &env_handle );
	FATAL_ASSERT( error_code == SQL_SUCCESS );

	error_code = SQLSetEnvAttr( env_handle, SQL_ATTR_ODBC_VERSION, (void*) SQL_OV_ODBC3, 0 );
	FATAL_ASSERT( error_code == SQL_SUCCESS );

	SQLWCHAR source_name[ 256 ];
	SQLWCHAR description[ 1024 ];
	SQLSMALLINT source_name_size = 0;
	SQLSMALLINT description_size = 0;

	while ( error_code != SQL_NO_DATA )
	{
		error_code = SQLDataSources( env_handle, SQL_FETCH_NEXT, source_name, sizeof( source_name ), &source_name_size, description, sizeof( description ), &description_size );
	}

	SQLFreeHandle( SQL_HANDLE_ENV, env_handle );
}

void Exception_Test_3( void )
{
	int *nullpointer = nullptr;
	*nullpointer = 5;
}

void Exception_Test_2( void )
{
	Exception_Test_3();
}

void Exception_Test_1( void )
{
	CLogInterface::Initialize_Dynamic( true );
	Exception_Test_2();
}

namespace NAuthServer
{
	void Initialize( void )
	{
		NServerShared::Initialize();
		Register_AuthServer_Enums();
	}

	void Shutdown( void )
	{
		NServerShared::Shutdown();
	}
}

int main( int /*argc*/, wchar_t* /*argv*/[] )
{
	NAuthServer::Initialize();

	Sql_Stuff2();
	//SQL_Stuff();
	//Exception_Test_1();

	NAuthServer::Shutdown();

	return 0;
}

