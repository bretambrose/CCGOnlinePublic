// AuthServer.cpp : Defines the entry point for the console application.
//
#include "stdafx.h"

#include "ServerShared.h"

#include <Windows.h>
#include <sql.h>
#include <sqlext.h>
#include <sqltypes.h>


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
											 (SQLWCHAR *)L"dsn=NetrunnerTest", 
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

int main( int /*argc*/, wchar_t* /*argv*/[] )
{
	NServerShared::Initialize();

	DEBUG_ASSERT( false );
	FATAL_ASSERT( false );

	//SQL_Stuff();
	//Exception_Test_1();

	NServerShared::Shutdown();

	return 0;
}

