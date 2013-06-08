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
#include "SlashCommands/SlashCommandManager.h"
#include "SlashCommands/SlashCommandInstance.h"
#include "Database/ODBCImplementation/ODBCFactory.h"
#include "Database/ODBCImplementation/ODBCParameters.h"
#include "Database/ODBCImplementation/ODBCVariableSet.h"
#include "Database/Interfaces/DatabaseVariableSetInterface.h"
#include "Database/Interfaces/DatabaseConnectionInterface.h"
#include "Database/Interfaces/DatabaseStatementInterface.h"
#include "Database/Interfaces/DatabaseEnvironmentInterface.h"
#include "Database/EmptyVariableSet.h"
#include "Database/DatabaseCalls.h"
#include "Database/DatabaseTaskBatch.h"

#include <iostream>

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

void Enumerate_Data_Sources( void )
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


#ifdef USE_ORACLE

struct SAddAccountInput
{
	SQLCHAR	EmailValue[ 256 ];
	SQLLEN EmailLengthIndicator;

	SQLCHAR	Nickname[ 33 ];
	SQLLEN NicknameLengthIndicator;

	SQLCHAR	PasswordHash[ 33 ];
	SQLLEN PasswordHashLengthIndicator;

};

struct STestGetResultSetInput
{
	SQLCHAR	EmailValue[ 256 ];
	SQLLEN EmailLengthIndicator;

};

struct STestGetResultSetOutput
{
	SQLCHAR AccountID[11];
	SQLLEN AccountIDLengthIndicator;

	SQLCHAR	Nickname[ 33 ];
	SQLLEN NicknameLengthIndicator;

	SQLUINTEGER NicknameSequenceID;
	SQLLEN NicknameSequenceIDLengthIndicator;
};

void Test_Oracle_Connection( void )
{
	SQLHENV env_handle = 0;
	SQLRETURN error_code = SQLAllocHandle( SQL_HANDLE_ENV, SQL_NULL_HANDLE, &env_handle );
	FATAL_ASSERT( error_code == SQL_SUCCESS );

	error_code = SQLSetEnvAttr( env_handle, SQL_ATTR_ODBC_VERSION, (void*) SQL_OV_ODBC3, 0 );
	FATAL_ASSERT( error_code == SQL_SUCCESS );

	SQLHDBC dbc_handle = 0;
	error_code = SQLAllocHandle( SQL_HANDLE_DBC, env_handle, &dbc_handle );
	FATAL_ASSERT( error_code == SQL_SUCCESS );

	SQLWCHAR output_connection_buffer[ 1024 ] = { 0 };
	SQLSMALLINT output_size = 0;

	error_code = SQLDriverConnect( dbc_handle, 
											 0, 
											 (SQLWCHAR *)L"Driver={Oracle in OraDb11g_home1};Dbq=ORCL;Uid=TESTSERVER;Pwd=TEST5erver;", 
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

void Enumerate_Drivers( void )
{
	SQLHENV env_handle = 0;
	SQLRETURN error_code = SQLAllocHandle( SQL_HANDLE_ENV, SQL_NULL_HANDLE, &env_handle );
	FATAL_ASSERT( error_code == SQL_SUCCESS );

	error_code = SQLSetEnvAttr( env_handle, SQL_ATTR_ODBC_VERSION, (void*) SQL_OV_ODBC3, 0 );
	FATAL_ASSERT( error_code == SQL_SUCCESS );

	SQLWCHAR attributes[ 1024 ];
	SQLWCHAR description[ 1024 ];
	SQLSMALLINT attributes_size = 0;
	SQLSMALLINT description_size = 0;

	while ( error_code != SQL_NO_DATA )
	{
		error_code = SQLDrivers( env_handle, SQL_FETCH_NEXT, description, sizeof( description ), &description_size, attributes, sizeof( attributes ), &attributes_size );
	}

	SQLFreeHandle( SQL_HANDLE_ENV, env_handle );
}


void Test_Oracle_Procedure_Call_Input_Only( void )
{
	SQLHENV env_handle = 0;
	SQLRETURN error_code = SQLAllocHandle( SQL_HANDLE_ENV, SQL_NULL_HANDLE, &env_handle );
	FATAL_ASSERT( error_code == SQL_SUCCESS );

	error_code = SQLSetEnvAttr( env_handle, SQL_ATTR_ODBC_VERSION, (void*) SQL_OV_ODBC3, 0 );
	FATAL_ASSERT( error_code == SQL_SUCCESS );

	SQLHDBC dbc_handle = 0;
	error_code = SQLAllocHandle( SQL_HANDLE_DBC, env_handle, &dbc_handle );
	FATAL_ASSERT( error_code == SQL_SUCCESS );

	SQLWCHAR output_connection_buffer[ 1024 ] = { 0 };
	SQLSMALLINT output_size = 0;

	error_code = SQLDriverConnect( dbc_handle, 
											 0, 
											 (SQLWCHAR *)L"Driver={Oracle in OraDb11g_home1};Dbq=ORCL;Uid=TESTSERVER;Pwd=TEST5erver;", 
											 SQL_NTS, 
											 output_connection_buffer, 
											 1024, 
											 &output_size, 
											 SQL_DRIVER_COMPLETE );
	FATAL_ASSERT( error_code == SQL_SUCCESS );

	// int32 auto_commit_value = SQL_AUTOCOMMIT_OFF;
	error_code = SQLSetConnectAttr( dbc_handle, SQL_ATTR_AUTOCOMMIT, (SQLPOINTER) SQL_AUTOCOMMIT_OFF, 0 );
	FATAL_ASSERT( error_code == SQL_SUCCESS );

	SQLHSTMT statement_handle = 0;
	error_code = SQLAllocHandle( SQL_HANDLE_STMT, dbc_handle, &statement_handle );
	FATAL_ASSERT( error_code == SQL_SUCCESS );

	SAddAccountInput test_input[ 2 ];
	strcpy_s( (char * )test_input[ 0 ].EmailValue, 256, "noob@noobsauce.com" );
	test_input[ 0 ].EmailLengthIndicator = SQL_NTS;
	strcpy_s( (char * )test_input[ 0 ].Nickname, 33, "Nooblar" );
	test_input[ 0 ].NicknameLengthIndicator = SQL_NTS;
	strcpy_s( (char * )test_input[ 0 ].PasswordHash, 33, "FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF" );
	test_input[ 0 ].PasswordHashLengthIndicator = SQL_NTS;

	strcpy_s( (char * )test_input[ 1 ].EmailValue, 256, "loser@loserville.edu" );
	test_input[ 1 ].EmailLengthIndicator = SQL_NTS;
	strcpy_s( (char * )test_input[ 1 ].Nickname, 33, "Loser" );
	test_input[ 1 ].NicknameLengthIndicator = SQL_NTS;
	strcpy_s( (char * )test_input[ 1 ].PasswordHash, 33, "F1FFFFFFFFFFFFFFFFFFFFFFFFFFFFFF" );
	test_input[ 1 ].PasswordHashLengthIndicator = SQL_NTS;

	error_code = SQLSetStmtAttr( statement_handle, SQL_ATTR_PARAM_BIND_TYPE, (SQLPOINTER) sizeof( SAddAccountInput ), 0 );
	FATAL_ASSERT( error_code == SQL_SUCCESS );

	error_code = SQLBindParameter( statement_handle, 1, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_CHAR, 256, 0, test_input[ 0 ].EmailValue, 256, &test_input[ 0 ].EmailLengthIndicator );
	FATAL_ASSERT( error_code == SQL_SUCCESS );

	error_code = SQLBindParameter( statement_handle, 2, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_CHAR, 33, 0, test_input[ 0 ].Nickname, 33, &test_input[ 0 ].NicknameLengthIndicator );
	FATAL_ASSERT( error_code == SQL_SUCCESS );

	error_code = SQLBindParameter( statement_handle, 3, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_CHAR, 33, 0, test_input[ 0 ].PasswordHash, 33, &test_input[ 0 ].PasswordHashLengthIndicator );
	FATAL_ASSERT( error_code == SQL_SUCCESS );

	error_code = SQLSetStmtAttr( statement_handle, SQL_ATTR_PARAMSET_SIZE, (SQLPOINTER) 2, 0 );
	FATAL_ASSERT( error_code == SQL_SUCCESS );

	std::wstring statement_text( L"{call test.add_account(?,?,?)}" );
	error_code = SQLExecDirect( statement_handle, (SQLWCHAR *)statement_text.c_str(), SQL_NTS );
	FATAL_ASSERT( error_code == SQL_SUCCESS );

	if ( error_code == SQL_SUCCESS )
	{
		error_code = SQLEndTran( SQL_HANDLE_DBC, dbc_handle, SQL_COMMIT );
		FATAL_ASSERT( error_code == SQL_SUCCESS );
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

	SQLFreeHandle( SQL_HANDLE_STMT, statement_handle );
	SQLDisconnect( dbc_handle );
	SQLFreeHandle( SQL_HANDLE_DBC, dbc_handle );
	SQLFreeHandle( SQL_HANDLE_ENV, env_handle );
}

struct STestDescribeResultSetInput
{
	SQLCHAR	EmailValue[ 256 ];
	SQLLEN EmailLengthIndicator;

};

void Test_Oracle_Procedure_Call_Analyze_Output( void )
{
	SQLHENV env_handle = 0;
	SQLRETURN error_code = SQLAllocHandle( SQL_HANDLE_ENV, SQL_NULL_HANDLE, &env_handle );
	FATAL_ASSERT( error_code == SQL_SUCCESS );

	error_code = SQLSetEnvAttr( env_handle, SQL_ATTR_ODBC_VERSION, (void*) SQL_OV_ODBC3, 0 );
	FATAL_ASSERT( error_code == SQL_SUCCESS );

	SQLHDBC dbc_handle = 0;
	error_code = SQLAllocHandle( SQL_HANDLE_DBC, env_handle, &dbc_handle );
	FATAL_ASSERT( error_code == SQL_SUCCESS );

	SQLWCHAR output_connection_buffer[ 1024 ] = { 0 };
	SQLSMALLINT output_size = 0;

	error_code = SQLDriverConnect( dbc_handle, 
											 0, 
											 (SQLWCHAR *)L"Driver={Oracle in OraDb11g_home1};Dbq=ORCL;Uid=TESTSERVER;Pwd=TEST5erver;", 
											 SQL_NTS, 
											 output_connection_buffer, 
											 1024, 
											 &output_size, 
											 SQL_DRIVER_COMPLETE );
	FATAL_ASSERT( error_code == SQL_SUCCESS );

	// int32 auto_commit_value = SQL_AUTOCOMMIT_OFF;
	error_code = SQLSetConnectAttr( dbc_handle, SQL_ATTR_AUTOCOMMIT, (SQLPOINTER) SQL_AUTOCOMMIT_OFF, 0 );
	FATAL_ASSERT( error_code == SQL_SUCCESS );

	SQLHSTMT statement_handle = 0;
	error_code = SQLAllocHandle( SQL_HANDLE_STMT, dbc_handle, &statement_handle );
	FATAL_ASSERT( error_code == SQL_SUCCESS );

	STestDescribeResultSetInput test_input[ 1 ];
	strcpy_s( (char * )test_input[ 0 ].EmailValue, 256, "bretambrose@gmail.com" );
	test_input[ 0 ].EmailLengthIndicator = SQL_NTS;

	error_code = SQLSetStmtAttr( statement_handle, SQL_ATTR_PARAM_BIND_TYPE, (SQLPOINTER) sizeof( STestDescribeResultSetInput ), 0 );
	FATAL_ASSERT( error_code == SQL_SUCCESS );

	error_code = SQLBindParameter( statement_handle, 1, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_CHAR, 256, 0, test_input[ 0 ].EmailValue, 256, &test_input[ 0 ].EmailLengthIndicator );
	FATAL_ASSERT( error_code == SQL_SUCCESS );

	error_code = SQLSetStmtAttr( statement_handle, SQL_ATTR_PARAMSET_SIZE, (SQLPOINTER) 1, 0 );
	FATAL_ASSERT( error_code == SQL_SUCCESS );

	std::wstring statement_text( L"{call test.get_account_by_email(?)}" );
	error_code = SQLExecDirect( statement_handle, (SQLWCHAR *)statement_text.c_str(), SQL_NTS );

	if ( error_code == SQL_SUCCESS )
	{
		SQLSMALLINT column_count = 0;
		error_code = SQLNumResultCols( statement_handle, &column_count );
		FATAL_ASSERT( error_code == SQL_SUCCESS );

		SQLWCHAR column_name[ 33 ];
		SQLSMALLINT column_name_length = 0;
		SQLSMALLINT column_type = 0;
		SQLULEN column_size = 0;
		SQLSMALLINT scale = 0;
		SQLSMALLINT nullable = 0;
		for ( int32 i = 1; i <= column_count; ++i )
		{
			error_code = SQLDescribeCol( statement_handle, (SQLUSMALLINT) i, column_name, sizeof( column_name ), &column_name_length, &column_type, &column_size, &scale, &nullable );
			FATAL_ASSERT( error_code == SQL_SUCCESS );
		}

		error_code = SQLEndTran( SQL_HANDLE_DBC, dbc_handle, SQL_COMMIT );
		FATAL_ASSERT( error_code == SQL_SUCCESS );
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

	SQLFreeHandle( SQL_HANDLE_STMT, statement_handle );
	SQLDisconnect( dbc_handle );
	SQLFreeHandle( SQL_HANDLE_DBC, dbc_handle );
	SQLFreeHandle( SQL_HANDLE_ENV, env_handle );
}



void Test_Oracle_Procedure_Call_Get_Output( void )
{
	SQLHENV env_handle = 0;
	SQLRETURN error_code = SQLAllocHandle( SQL_HANDLE_ENV, SQL_NULL_HANDLE, &env_handle );
	FATAL_ASSERT( error_code == SQL_SUCCESS );

	error_code = SQLSetEnvAttr( env_handle, SQL_ATTR_ODBC_VERSION, (void*) SQL_OV_ODBC3, 0 );
	FATAL_ASSERT( error_code == SQL_SUCCESS );

	SQLHDBC dbc_handle = 0;
	error_code = SQLAllocHandle( SQL_HANDLE_DBC, env_handle, &dbc_handle );
	FATAL_ASSERT( error_code == SQL_SUCCESS );

	SQLWCHAR output_connection_buffer[ 1024 ] = { 0 };
	SQLSMALLINT output_size = 0;

	error_code = SQLDriverConnect( dbc_handle, 
											 0, 
											 (SQLWCHAR *)L"Driver={Oracle in OraDb11g_home1};Dbq=ORCL;Uid=TESTSERVER;Pwd=TEST5erver;", 
											 SQL_NTS, 
											 output_connection_buffer, 
											 1024, 
											 &output_size, 
											 SQL_DRIVER_COMPLETE );
	FATAL_ASSERT( error_code == SQL_SUCCESS );

	error_code = SQLSetConnectAttr( dbc_handle, SQL_ATTR_AUTOCOMMIT, (SQLPOINTER) SQL_AUTOCOMMIT_OFF, 0 );
	FATAL_ASSERT( error_code == SQL_SUCCESS );

	SQLHSTMT statement_handle = 0;
	error_code = SQLAllocHandle( SQL_HANDLE_STMT, dbc_handle, &statement_handle );
	FATAL_ASSERT( error_code == SQL_SUCCESS );

	STestGetResultSetInput test_input[ 1 ];
	strcpy_s( (char * )test_input[ 0 ].EmailValue, 256, "bretambrose@gmail.com" );
	test_input[ 0 ].EmailLengthIndicator = SQL_NTS;

	error_code = SQLSetStmtAttr( statement_handle, SQL_ATTR_PARAM_BIND_TYPE, (SQLPOINTER) sizeof( STestGetResultSetInput ), 0 );
	FATAL_ASSERT( error_code == SQL_SUCCESS );

	error_code = SQLBindParameter( statement_handle, 1, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_CHAR, 256, 0, test_input[ 0 ].EmailValue, 256, &test_input[ 0 ].EmailLengthIndicator );
	FATAL_ASSERT( error_code == SQL_SUCCESS );

	error_code = SQLSetStmtAttr( statement_handle, SQL_ATTR_PARAMSET_SIZE, (SQLPOINTER) 1, 0 );
	FATAL_ASSERT( error_code == SQL_SUCCESS );

	std::wstring statement_text( L"{call test.get_account_by_email(?)}" );
	error_code = SQLExecDirect( statement_handle, (SQLWCHAR *)statement_text.c_str(), SQL_NTS );
	FATAL_ASSERT( error_code == SQL_SUCCESS );

	STestGetResultSetOutput test_output[ 3 ];
	SQLLEN row_statuses[ 3 ];

	error_code = SQLSetStmtAttr( statement_handle, SQL_ATTR_ROW_BIND_TYPE, (SQLPOINTER) sizeof( STestGetResultSetOutput ), 0 );
	FATAL_ASSERT( error_code == SQL_SUCCESS );

	error_code = SQLSetStmtAttr( statement_handle, SQL_ATTR_ROW_ARRAY_SIZE, (SQLPOINTER) 3, 0 );
	FATAL_ASSERT( error_code == SQL_SUCCESS );

	error_code = SQLSetStmtAttr( statement_handle, SQL_ATTR_ROW_STATUS_PTR, row_statuses, 0 );
	FATAL_ASSERT( error_code == SQL_SUCCESS );

	int64 rows_fetched = 0;
	error_code = SQLSetStmtAttr( statement_handle, SQL_ATTR_ROWS_FETCHED_PTR, (SQLPOINTER) &rows_fetched, 0 );
	FATAL_ASSERT( error_code == SQL_SUCCESS );

	error_code = SQLBindCol( statement_handle, 1, SQL_C_CHAR, test_output[ 0 ].AccountID, 17, &test_output[ 0 ].AccountIDLengthIndicator );
	FATAL_ASSERT( error_code == SQL_SUCCESS );

	error_code = SQLBindCol( statement_handle, 2, SQL_C_CHAR, test_output[ 0 ].Nickname, 33, &test_output[ 0 ].NicknameLengthIndicator );
	FATAL_ASSERT( error_code == SQL_SUCCESS );

	error_code = SQLBindCol( statement_handle, 3, SQL_C_ULONG, &test_output[ 0 ].NicknameSequenceID, 0, &test_output[ 0 ].NicknameSequenceIDLengthIndicator );
	FATAL_ASSERT( error_code == SQL_SUCCESS );

	while ( error_code == SQL_SUCCESS )
	{
		error_code = SQLFetchScroll( statement_handle, SQL_FETCH_NEXT, 0 );
	}

	if ( error_code == SQL_NO_DATA_FOUND )
	{
		error_code = SQLEndTran( SQL_HANDLE_DBC, dbc_handle, SQL_COMMIT );
		FATAL_ASSERT( error_code == SQL_SUCCESS );
	}
	else
	{
		SQLSMALLINT text_length = 0;
		SQLINTEGER sql_error_code = 0;
		SQLWCHAR error_buffer[ 1024 ] = { 0 };
		SQLWCHAR sql_state[ 6 ] = { 0 };

		error_code = SQLError( env_handle, dbc_handle, statement_handle, sql_state, &sql_error_code, error_buffer, 1024, &text_length );
		while ( error_code == SQL_SUCCESS )
		{
			error_code = SQLError( env_handle, dbc_handle, statement_handle, sql_state, &sql_error_code, error_buffer, 1024, &text_length );
		}
	}

	SQLFreeHandle( SQL_HANDLE_STMT, statement_handle );
	SQLDisconnect( dbc_handle );
	SQLFreeHandle( SQL_HANDLE_DBC, dbc_handle );
	SQLFreeHandle( SQL_HANDLE_ENV, env_handle );
}

void Test_Oracle_Procedure_Call_Multi_Output( void )
{
	SQLHENV env_handle = 0;
	SQLRETURN error_code = SQLAllocHandle( SQL_HANDLE_ENV, SQL_NULL_HANDLE, &env_handle );
	FATAL_ASSERT( error_code == SQL_SUCCESS );

	error_code = SQLSetEnvAttr( env_handle, SQL_ATTR_ODBC_VERSION, (void*) SQL_OV_ODBC3, 0 );
	FATAL_ASSERT( error_code == SQL_SUCCESS );

	SQLHDBC dbc_handle = 0;
	error_code = SQLAllocHandle( SQL_HANDLE_DBC, env_handle, &dbc_handle );
	FATAL_ASSERT( error_code == SQL_SUCCESS );

	SQLWCHAR output_connection_buffer[ 1024 ] = { 0 };
	SQLSMALLINT output_size = 0;

	error_code = SQLDriverConnect( dbc_handle, 
											 0, 
											 (SQLWCHAR *)L"Driver={Oracle in OraDb11g_home1};Dbq=ORCL;Uid=TESTSERVER;Pwd=TEST5erver;", 
											 SQL_NTS, 
											 output_connection_buffer, 
											 1024, 
											 &output_size, 
											 SQL_DRIVER_COMPLETE );
	FATAL_ASSERT( error_code == SQL_SUCCESS );

	error_code = SQLSetConnectAttr( dbc_handle, SQL_ATTR_AUTOCOMMIT, (SQLPOINTER) SQL_AUTOCOMMIT_OFF, 0 );
	FATAL_ASSERT( error_code == SQL_SUCCESS );

	SQLHSTMT statement_handle = 0;
	error_code = SQLAllocHandle( SQL_HANDLE_STMT, dbc_handle, &statement_handle );
	FATAL_ASSERT( error_code == SQL_SUCCESS );

	STestGetResultSetInput test_input[ 2 ];
	strcpy_s( (char * )test_input[ 0 ].EmailValue, 256, "bretambrose@gmail.com" );
	test_input[ 0 ].EmailLengthIndicator = SQL_NTS;

	strcpy_s( (char * )test_input[ 1 ].EmailValue, 256, "petra222@yahoo.com" );
	test_input[ 1 ].EmailLengthIndicator = SQL_NTS;

	error_code = SQLSetStmtAttr( statement_handle, SQL_ATTR_PARAM_BIND_TYPE, (SQLPOINTER) sizeof( STestGetResultSetInput ), 0 );
	FATAL_ASSERT( error_code == SQL_SUCCESS );

	error_code = SQLBindParameter( statement_handle, 1, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_CHAR, 256, 0, test_input[ 0 ].EmailValue, 256, &test_input[ 0 ].EmailLengthIndicator );
	FATAL_ASSERT( error_code == SQL_SUCCESS );

	error_code = SQLSetStmtAttr( statement_handle, SQL_ATTR_PARAMSET_SIZE, (SQLPOINTER) 2, 0 );
	FATAL_ASSERT( error_code == SQL_SUCCESS );

	std::wstring statement_text( L"{call test.get_account_by_email(?)}" );
	error_code = SQLExecDirect( statement_handle, (SQLWCHAR *)statement_text.c_str(), SQL_NTS );
	FATAL_ASSERT( error_code == SQL_SUCCESS );

	STestGetResultSetOutput test_output[ 3 ];
	SQLLEN row_statuses[ 3 ];

	error_code = SQLSetStmtAttr( statement_handle, SQL_ATTR_ROW_BIND_TYPE, (SQLPOINTER) sizeof( STestGetResultSetOutput ), 0 );
	FATAL_ASSERT( error_code == SQL_SUCCESS );

	error_code = SQLSetStmtAttr( statement_handle, SQL_ATTR_ROW_ARRAY_SIZE, (SQLPOINTER) 3, 0 );
	FATAL_ASSERT( error_code == SQL_SUCCESS );

	error_code = SQLSetStmtAttr( statement_handle, SQL_ATTR_ROW_STATUS_PTR, row_statuses, 0 );
	FATAL_ASSERT( error_code == SQL_SUCCESS );

	int64 rows_fetched = 0;
	error_code = SQLSetStmtAttr( statement_handle, SQL_ATTR_ROWS_FETCHED_PTR, (SQLPOINTER) &rows_fetched, 0 );
	FATAL_ASSERT( error_code == SQL_SUCCESS );

	error_code = SQLBindCol( statement_handle, 1, SQL_C_CHAR, test_output[ 0 ].AccountID, 17, &test_output[ 0 ].AccountIDLengthIndicator );
	FATAL_ASSERT( error_code == SQL_SUCCESS );

	error_code = SQLBindCol( statement_handle, 2, SQL_C_CHAR, test_output[ 0 ].Nickname, 33, &test_output[ 0 ].NicknameLengthIndicator );
	FATAL_ASSERT( error_code == SQL_SUCCESS );

	error_code = SQLBindCol( statement_handle, 3, SQL_C_ULONG, &test_output[ 0 ].NicknameSequenceID, 0, &test_output[ 0 ].NicknameSequenceIDLengthIndicator );
	FATAL_ASSERT( error_code == SQL_SUCCESS );

	SQLRETURN ec2 = SQL_SUCCESS;
	while ( ec2 == SQL_SUCCESS )
	{
		while ( error_code == SQL_SUCCESS )
		{
			error_code = SQLFetchScroll( statement_handle, SQL_FETCH_NEXT, 0 );
		}

		if ( error_code == SQL_NO_DATA )
		{
			ec2 = SQLMoreResults( statement_handle );
		}
		else
		{
			SQLSMALLINT text_length = 0;
			SQLINTEGER sql_error_code = 0;
			SQLWCHAR error_buffer[ 1024 ] = { 0 };
			SQLWCHAR sql_state[ 6 ] = { 0 };

			error_code = SQLError( env_handle, dbc_handle, statement_handle, sql_state, &sql_error_code, error_buffer, 1024, &text_length );
			while ( error_code == SQL_SUCCESS )
			{
				error_code = SQLError( env_handle, dbc_handle, statement_handle, sql_state, &sql_error_code, error_buffer, 1024, &text_length );
			}

			ec2 = SQL_ERROR;
		}
	}

	if ( ec2 == SQL_NO_DATA_FOUND )
	{
		error_code = SQLEndTran( SQL_HANDLE_DBC, dbc_handle, SQL_COMMIT );
		FATAL_ASSERT( error_code == SQL_SUCCESS );
	}

	SQLFreeHandle( SQL_HANDLE_STMT, statement_handle );
	SQLDisconnect( dbc_handle );
	SQLFreeHandle( SQL_HANDLE_DBC, dbc_handle );
	SQLFreeHandle( SQL_HANDLE_ENV, env_handle );
}

#else

void Test_Sql_Server_Connection( void )
{
	SQLHENV env_handle = 0;
	SQLRETURN error_code = SQLAllocHandle( SQL_HANDLE_ENV, SQL_NULL_HANDLE, &env_handle );
	FATAL_ASSERT( error_code == SQL_SUCCESS );

	error_code = SQLSetEnvAttr( env_handle, SQL_ATTR_ODBC_VERSION, (void*) SQL_OV_ODBC3, 0 );
	FATAL_ASSERT( error_code == SQL_SUCCESS );

	SQLHDBC dbc_handle = 0;
	error_code = SQLAllocHandle( SQL_HANDLE_DBC, env_handle, &dbc_handle );
	FATAL_ASSERT( error_code == SQL_SUCCESS );

	SQLWCHAR output_connection_buffer[ 1024 ] = { 0 };
	SQLSMALLINT output_size = 0;

	SQLWCHAR *connection_string = (SQLWCHAR *)L"Driver={SQL Server Native Client 11.0};Server=AZAZELPC\\CCGONLINE;Database=testdb;UID=testserver;PWD=TEST5erver#;";

	error_code = SQLDriverConnect( dbc_handle, 
											 0, 
											 connection_string, 
											 SQL_NTS, 
											 output_connection_buffer, 
											 1024, 
											 &output_size, 
											 SQL_DRIVER_COMPLETE );
	if ( error_code == SQL_SUCCESS || error_code == SQL_SUCCESS_WITH_INFO )
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

struct SAddAccountInput
{
	SQLCHAR	EmailValue[ 256 ];
	SQLLEN EmailLengthIndicator;

	SQLCHAR	Nickname[ 33 ];
	SQLLEN NicknameLengthIndicator;

	SQLCHAR	PasswordHash[ 33 ];
	SQLLEN PasswordHashLengthIndicator;

};

void Test_SQL_Server_Input_Only( void )
{
	SQLHENV env_handle = 0;
	SQLRETURN error_code = SQLAllocHandle( SQL_HANDLE_ENV, SQL_NULL_HANDLE, &env_handle );
	FATAL_ASSERT( error_code == SQL_SUCCESS );

	error_code = SQLSetEnvAttr( env_handle, SQL_ATTR_ODBC_VERSION, (void*) SQL_OV_ODBC3, 0 );
	FATAL_ASSERT( error_code == SQL_SUCCESS );

	SQLHDBC dbc_handle = 0;
	error_code = SQLAllocHandle( SQL_HANDLE_DBC, env_handle, &dbc_handle );
	FATAL_ASSERT( error_code == SQL_SUCCESS );

	SQLWCHAR output_connection_buffer[ 1024 ] = { 0 };
	SQLSMALLINT output_size = 0;

	error_code = SQLDriverConnect( dbc_handle, 
											 0, 
											 (SQLWCHAR *)L"Driver={SQL Server Native Client 11.0};Server=AZAZELPC\\CCGONLINE;Database=testdb;UID=testserver;PWD=TEST5erver#;", 
											 SQL_NTS, 
											 output_connection_buffer, 
											 1024, 
											 &output_size, 
											 SQL_DRIVER_COMPLETE );
	FATAL_ASSERT( error_code == SQL_SUCCESS ||error_code == SQL_SUCCESS_WITH_INFO );

	error_code = SQLSetConnectAttr( dbc_handle, SQL_ATTR_AUTOCOMMIT, (SQLPOINTER) SQL_AUTOCOMMIT_OFF, 0 );
	FATAL_ASSERT( error_code == SQL_SUCCESS );

	SQLHSTMT statement_handle = 0;
	error_code = SQLAllocHandle( SQL_HANDLE_STMT, dbc_handle, &statement_handle );
	FATAL_ASSERT( error_code == SQL_SUCCESS );

	SAddAccountInput test_input[ 2 ];
	strcpy_s( (char * )test_input[ 0 ].EmailValue, 256, "noob@noobsauce.com" );
	test_input[ 0 ].EmailLengthIndicator = SQL_NTS;
	strcpy_s( (char * )test_input[ 0 ].Nickname, 33, "Nooblar" );
	test_input[ 0 ].NicknameLengthIndicator = SQL_NTS;
	strcpy_s( (char * )test_input[ 0 ].PasswordHash, 33, "FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF" );
	test_input[ 0 ].PasswordHashLengthIndicator = SQL_NTS;

	strcpy_s( (char * )test_input[ 1 ].EmailValue, 256, "loser@loserville.edu" );
	test_input[ 1 ].EmailLengthIndicator = SQL_NTS;
	strcpy_s( (char * )test_input[ 1 ].Nickname, 33, "Loser" );
	test_input[ 1 ].NicknameLengthIndicator = SQL_NTS;
	strcpy_s( (char * )test_input[ 1 ].PasswordHash, 33, "F1FFFFFFFFFFFFFFFFFFFFFFFFFFFFFF" );
	test_input[ 1 ].PasswordHashLengthIndicator = SQL_NTS;

	error_code = SQLSetStmtAttr( statement_handle, SQL_ATTR_PARAM_BIND_TYPE, (SQLPOINTER) sizeof( SAddAccountInput ), 0 );
	FATAL_ASSERT( error_code == SQL_SUCCESS );

	error_code = SQLBindParameter( statement_handle, 1, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_CHAR, 256, 0, test_input[ 0 ].EmailValue, 256, &test_input[ 0 ].EmailLengthIndicator );
	FATAL_ASSERT( error_code == SQL_SUCCESS );

	error_code = SQLBindParameter( statement_handle, 2, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_CHAR, 33, 0, test_input[ 0 ].Nickname, 33, &test_input[ 0 ].NicknameLengthIndicator );
	FATAL_ASSERT( error_code == SQL_SUCCESS );

	error_code = SQLBindParameter( statement_handle, 3, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_CHAR, 33, 0, test_input[ 0 ].PasswordHash, 33, &test_input[ 0 ].PasswordHashLengthIndicator );
	FATAL_ASSERT( error_code == SQL_SUCCESS );

	error_code = SQLSetStmtAttr( statement_handle, SQL_ATTR_PARAMSET_SIZE, (SQLPOINTER) 2, 0 );
	FATAL_ASSERT( error_code == SQL_SUCCESS );

	std::wstring statement_text( L"{call dynamic.add_account(?,?,?)}" );
	error_code = SQLExecDirect( statement_handle, (SQLWCHAR *)statement_text.c_str(), SQL_NTS );
	FATAL_ASSERT( error_code == SQL_SUCCESS );

	if ( error_code == SQL_SUCCESS )
	{
		error_code = SQLEndTran( SQL_HANDLE_DBC, dbc_handle, SQL_COMMIT );
		FATAL_ASSERT( error_code == SQL_SUCCESS );
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

	SQLFreeHandle( SQL_HANDLE_STMT, statement_handle );
	SQLDisconnect( dbc_handle );
	SQLFreeHandle( SQL_HANDLE_DBC, dbc_handle );
	SQLFreeHandle( SQL_HANDLE_ENV, env_handle );
}

struct STestGetResultSetInput
{
	SQLCHAR	EmailValue[ 256 ];
	SQLLEN EmailLengthIndicator;

};

struct STestGetResultSetOutput
{
	SQLUBIGINT AccountID;
	SQLLEN AccountIDLengthIndicator;

	SQLCHAR	Nickname[ 33 ];
	SQLLEN NicknameLengthIndicator;

	SQLUINTEGER NicknameSequenceID;
	SQLLEN NicknameSequenceIDLengthIndicator;
};

void Test_SQL_Server_Single_Result_Set( void )
{
	SQLHENV env_handle = 0;
	SQLRETURN error_code = SQLAllocHandle( SQL_HANDLE_ENV, SQL_NULL_HANDLE, &env_handle );
	FATAL_ASSERT( error_code == SQL_SUCCESS );

	error_code = SQLSetEnvAttr( env_handle, SQL_ATTR_ODBC_VERSION, (void*) SQL_OV_ODBC3, 0 );
	FATAL_ASSERT( error_code == SQL_SUCCESS );

	SQLHDBC dbc_handle = 0;
	error_code = SQLAllocHandle( SQL_HANDLE_DBC, env_handle, &dbc_handle );
	FATAL_ASSERT( error_code == SQL_SUCCESS );

	SQLWCHAR output_connection_buffer[ 1024 ] = { 0 };
	SQLSMALLINT output_size = 0;

	error_code = SQLDriverConnect( dbc_handle, 
											 0, 
											 (SQLWCHAR *)L"Driver={SQL Server Native Client 11.0};Server=AZAZELPC\\CCGONLINE;Database=testdb;UID=testserver;PWD=TEST5erver#;", 
											 SQL_NTS, 
											 output_connection_buffer, 
											 1024, 
											 &output_size, 
											 SQL_DRIVER_COMPLETE );
	FATAL_ASSERT( error_code == SQL_SUCCESS ||error_code == SQL_SUCCESS_WITH_INFO );

	error_code = SQLSetConnectAttr( dbc_handle, SQL_ATTR_AUTOCOMMIT, (SQLPOINTER) SQL_AUTOCOMMIT_OFF, 0 );
	FATAL_ASSERT( error_code == SQL_SUCCESS );

	SQLHSTMT statement_handle = 0;
	error_code = SQLAllocHandle( SQL_HANDLE_STMT, dbc_handle, &statement_handle );
	FATAL_ASSERT( error_code == SQL_SUCCESS );

	STestGetResultSetInput test_input[ 1 ];
	strcpy_s( (char * )test_input[ 0 ].EmailValue, 256, "bretambrose@gmail.com" );
	test_input[ 0 ].EmailLengthIndicator = SQL_NTS;

	error_code = SQLSetStmtAttr( statement_handle, SQL_ATTR_PARAM_BIND_TYPE, (SQLPOINTER) sizeof( STestGetResultSetInput ), 0 );
	FATAL_ASSERT( error_code == SQL_SUCCESS );

	error_code = SQLBindParameter( statement_handle, 1, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_CHAR, 256, 0, test_input[ 0 ].EmailValue, 256, &test_input[ 0 ].EmailLengthIndicator );
	FATAL_ASSERT( error_code == SQL_SUCCESS );

	error_code = SQLSetStmtAttr( statement_handle, SQL_ATTR_PARAMSET_SIZE, (SQLPOINTER) 1, 0 );
	FATAL_ASSERT( error_code == SQL_SUCCESS );

	std::wstring statement_text( L"{call dynamic.get_account_by_email(?)}" );
	error_code = SQLExecDirect( statement_handle, (SQLWCHAR *)statement_text.c_str(), SQL_NTS );
	FATAL_ASSERT( error_code == SQL_SUCCESS );

	STestGetResultSetOutput test_output[ 3 ];
	SQLSMALLINT row_statuses[ 3 ];

	error_code = SQLSetStmtAttr( statement_handle, SQL_ATTR_ROW_BIND_TYPE, (SQLPOINTER) sizeof( STestGetResultSetOutput ), 0 );
	FATAL_ASSERT( error_code == SQL_SUCCESS );

	error_code = SQLSetStmtAttr( statement_handle, SQL_ATTR_ROW_ARRAY_SIZE, (SQLPOINTER) 3, 0 );
	FATAL_ASSERT( error_code == SQL_SUCCESS );

	error_code = SQLSetStmtAttr( statement_handle, SQL_ATTR_ROW_STATUS_PTR, row_statuses, 0 );
	FATAL_ASSERT( error_code == SQL_SUCCESS );

	int64 rows_fetched = 0;
	error_code = SQLSetStmtAttr( statement_handle, SQL_ATTR_ROWS_FETCHED_PTR, (SQLPOINTER) &rows_fetched, 0 );
	FATAL_ASSERT( error_code == SQL_SUCCESS );

	error_code = SQLBindCol( statement_handle, 1, SQL_C_UBIGINT, &test_output[ 0 ].AccountID, 0, &test_output[ 0 ].AccountIDLengthIndicator );
	FATAL_ASSERT( error_code == SQL_SUCCESS );

	error_code = SQLBindCol( statement_handle, 2, SQL_C_CHAR, test_output[ 0 ].Nickname, 33, &test_output[ 0 ].NicknameLengthIndicator );
	FATAL_ASSERT( error_code == SQL_SUCCESS );

	error_code = SQLBindCol( statement_handle, 3, SQL_C_ULONG, &test_output[ 0 ].NicknameSequenceID, 0, &test_output[ 0 ].NicknameSequenceIDLengthIndicator );
	FATAL_ASSERT( error_code == SQL_SUCCESS );

	while ( error_code == SQL_SUCCESS )
	{
		error_code = SQLFetchScroll( statement_handle, SQL_FETCH_NEXT, 0 );
	}

	if ( error_code == SQL_NO_DATA_FOUND )
	{
		error_code = SQLEndTran( SQL_HANDLE_DBC, dbc_handle, SQL_COMMIT );
		FATAL_ASSERT( error_code == SQL_SUCCESS );
	}
	else
	{
		SQLSMALLINT text_length = 0;
		SQLINTEGER sql_error_code = 0;
		SQLWCHAR error_buffer[ 1024 ] = { 0 };
		SQLWCHAR sql_state[ 6 ] = { 0 };

		error_code = SQLError( env_handle, dbc_handle, statement_handle, sql_state, &sql_error_code, error_buffer, 1024, &text_length );
		while ( error_code == SQL_SUCCESS )
		{
			error_code = SQLError( env_handle, dbc_handle, statement_handle, sql_state, &sql_error_code, error_buffer, 1024, &text_length );
		}
	}

	SQLFreeHandle( SQL_HANDLE_STMT, statement_handle );
	SQLDisconnect( dbc_handle );
	SQLFreeHandle( SQL_HANDLE_DBC, dbc_handle );
	SQLFreeHandle( SQL_HANDLE_ENV, env_handle );
}

void Test_SQL_Server_Multi_Result_Set( void )
{
	SQLHENV env_handle = 0;
	SQLRETURN error_code = SQLAllocHandle( SQL_HANDLE_ENV, SQL_NULL_HANDLE, &env_handle );
	FATAL_ASSERT( error_code == SQL_SUCCESS );

	error_code = SQLSetEnvAttr( env_handle, SQL_ATTR_ODBC_VERSION, (void*) SQL_OV_ODBC3, 0 );
	FATAL_ASSERT( error_code == SQL_SUCCESS );

	SQLHDBC dbc_handle = 0;
	error_code = SQLAllocHandle( SQL_HANDLE_DBC, env_handle, &dbc_handle );
	FATAL_ASSERT( error_code == SQL_SUCCESS );

	SQLWCHAR output_connection_buffer[ 1024 ] = { 0 };
	SQLSMALLINT output_size = 0;

	error_code = SQLDriverConnect( dbc_handle, 
											 0, 
											 (SQLWCHAR *)L"Driver={SQL Server Native Client 11.0};Server=AZAZELPC\\CCGONLINE;Database=testdb;UID=testserver;PWD=TEST5erver#;", 
											 SQL_NTS, 
											 output_connection_buffer, 
											 1024, 
											 &output_size, 
											 SQL_DRIVER_COMPLETE );
	FATAL_ASSERT( error_code == SQL_SUCCESS ||error_code == SQL_SUCCESS_WITH_INFO );

	error_code = SQLSetConnectAttr( dbc_handle, SQL_ATTR_AUTOCOMMIT, (SQLPOINTER) SQL_AUTOCOMMIT_OFF, 0 );
	FATAL_ASSERT( error_code == SQL_SUCCESS );

	SQLHSTMT statement_handle = 0;
	error_code = SQLAllocHandle( SQL_HANDLE_STMT, dbc_handle, &statement_handle );
	FATAL_ASSERT( error_code == SQL_SUCCESS );

	STestGetResultSetInput test_input[ 2 ];
	strcpy_s( (char * )test_input[ 0 ].EmailValue, 256, "bretambrose@gmail.com" );
	test_input[ 0 ].EmailLengthIndicator = SQL_NTS;

	strcpy_s( (char * )test_input[ 1 ].EmailValue, 256, "petra222@yahoo.com" );
	test_input[ 1 ].EmailLengthIndicator = SQL_NTS;

	error_code = SQLSetStmtAttr( statement_handle, SQL_ATTR_PARAM_BIND_TYPE, (SQLPOINTER) sizeof( STestGetResultSetInput ), 0 );
	FATAL_ASSERT( error_code == SQL_SUCCESS );

	error_code = SQLBindParameter( statement_handle, 1, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_CHAR, 256, 0, test_input[ 0 ].EmailValue, 256, &test_input[ 0 ].EmailLengthIndicator );
	FATAL_ASSERT( error_code == SQL_SUCCESS );

	error_code = SQLSetStmtAttr( statement_handle, SQL_ATTR_PARAMSET_SIZE, (SQLPOINTER) 2, 0 );
	FATAL_ASSERT( error_code == SQL_SUCCESS );

	std::wstring statement_text( L"{call dynamic.get_account_by_email(?)}" );
	error_code = SQLExecDirect( statement_handle, (SQLWCHAR *)statement_text.c_str(), SQL_NTS );
	FATAL_ASSERT( error_code == SQL_SUCCESS );

	STestGetResultSetOutput test_output[ 3 ];
	SQLSMALLINT row_statuses[ 3 ];

	error_code = SQLSetStmtAttr( statement_handle, SQL_ATTR_ROW_BIND_TYPE, (SQLPOINTER) sizeof( STestGetResultSetOutput ), 0 );
	FATAL_ASSERT( error_code == SQL_SUCCESS );

	error_code = SQLSetStmtAttr( statement_handle, SQL_ATTR_ROW_ARRAY_SIZE, (SQLPOINTER) 3, 0 );
	FATAL_ASSERT( error_code == SQL_SUCCESS );

	error_code = SQLSetStmtAttr( statement_handle, SQL_ATTR_ROW_STATUS_PTR, row_statuses, 0 );
	FATAL_ASSERT( error_code == SQL_SUCCESS );

	int64 rows_fetched = 0;
	error_code = SQLSetStmtAttr( statement_handle, SQL_ATTR_ROWS_FETCHED_PTR, (SQLPOINTER) &rows_fetched, 0 );
	FATAL_ASSERT( error_code == SQL_SUCCESS );

	error_code = SQLBindCol( statement_handle, 1, SQL_C_UBIGINT, &test_output[ 0 ].AccountID, 0, &test_output[ 0 ].AccountIDLengthIndicator );
	FATAL_ASSERT( error_code == SQL_SUCCESS );

	error_code = SQLBindCol( statement_handle, 2, SQL_C_CHAR, test_output[ 0 ].Nickname, 33, &test_output[ 0 ].NicknameLengthIndicator );
	FATAL_ASSERT( error_code == SQL_SUCCESS );

	error_code = SQLBindCol( statement_handle, 3, SQL_C_ULONG, &test_output[ 0 ].NicknameSequenceID, 0, &test_output[ 0 ].NicknameSequenceIDLengthIndicator );
	FATAL_ASSERT( error_code == SQL_SUCCESS );

	SQLRETURN ec2 = SQL_SUCCESS;
	while ( ec2 == SQL_SUCCESS )
	{
		while ( error_code == SQL_SUCCESS )
		{
			error_code = SQLFetchScroll( statement_handle, SQL_FETCH_NEXT, 0 );
		}

		if ( error_code == SQL_NO_DATA )
		{
			ec2 = SQLMoreResults( statement_handle );
			error_code = SQL_SUCCESS;
		}
		else
		{
			SQLSMALLINT text_length = 0;
			SQLINTEGER sql_error_code = 0;
			SQLWCHAR error_buffer[ 1024 ] = { 0 };
			SQLWCHAR sql_state[ 6 ] = { 0 };

			error_code = SQLError( env_handle, dbc_handle, statement_handle, sql_state, &sql_error_code, error_buffer, 1024, &text_length );
			while ( error_code == SQL_SUCCESS )
			{
				error_code = SQLError( env_handle, dbc_handle, statement_handle, sql_state, &sql_error_code, error_buffer, 1024, &text_length );
			}

			ec2 = SQL_ERROR;
		}
	}

	if ( ec2 == SQL_NO_DATA_FOUND )
	{
		error_code = SQLEndTran( SQL_HANDLE_DBC, dbc_handle, SQL_COMMIT );
		FATAL_ASSERT( error_code == SQL_SUCCESS );
	}

	SQLFreeHandle( SQL_HANDLE_STMT, statement_handle );
	SQLDisconnect( dbc_handle );
	SQLFreeHandle( SQL_HANDLE_DBC, dbc_handle );
	SQLFreeHandle( SQL_HANDLE_ENV, env_handle );
}

struct STestFunctionInput
{
	SQLUBIGINT AccountID;
	SQLLEN AccountIDLengthIndicator;

	SQLCHAR	EmailValue[ 256 ];
	SQLLEN EmailLengthIndicator;
};

struct SNullOutput
{
};

void Test_SQL_Server_Function_No_Results( void )
{
	SQLHENV env_handle = 0;
	SQLRETURN error_code = SQLAllocHandle( SQL_HANDLE_ENV, SQL_NULL_HANDLE, &env_handle );
	FATAL_ASSERT( error_code == SQL_SUCCESS );

	error_code = SQLSetEnvAttr( env_handle, SQL_ATTR_ODBC_VERSION, (void*) SQL_OV_ODBC3, 0 );
	FATAL_ASSERT( error_code == SQL_SUCCESS );

	SQLHDBC dbc_handle = 0;
	error_code = SQLAllocHandle( SQL_HANDLE_DBC, env_handle, &dbc_handle );
	FATAL_ASSERT( error_code == SQL_SUCCESS );

	SQLWCHAR output_connection_buffer[ 1024 ] = { 0 };
	SQLSMALLINT output_size = 0;

	error_code = SQLDriverConnect( dbc_handle, 
											 0, 
											 (SQLWCHAR *)L"Driver={SQL Server Native Client 11.0};Server=AZAZELPC\\CCGONLINE;Database=testdb;UID=testserver;PWD=TEST5erver#;", 
											 SQL_NTS, 
											 output_connection_buffer, 
											 1024, 
											 &output_size, 
											 SQL_DRIVER_COMPLETE );
	FATAL_ASSERT( error_code == SQL_SUCCESS ||error_code == SQL_SUCCESS_WITH_INFO );

	error_code = SQLSetConnectAttr( dbc_handle, SQL_ATTR_AUTOCOMMIT, (SQLPOINTER) SQL_AUTOCOMMIT_OFF, 0 );
	FATAL_ASSERT( error_code == SQL_SUCCESS );

	SQLHSTMT statement_handle = 0;
	error_code = SQLAllocHandle( SQL_HANDLE_STMT, dbc_handle, &statement_handle );
	FATAL_ASSERT( error_code == SQL_SUCCESS );

	STestFunctionInput test_input[ 2 ];
	strcpy_s( (char * )test_input[ 0 ].EmailValue, 256, "bretambrose@gmail.com" );
	test_input[ 0 ].EmailLengthIndicator = SQL_NTS;

	strcpy_s( (char * )test_input[ 1 ].EmailValue, 256, "petra222@yahoo.com" );
	test_input[ 1 ].EmailLengthIndicator = SQL_NTS;

	error_code = SQLSetStmtAttr( statement_handle, SQL_ATTR_PARAM_BIND_TYPE, (SQLPOINTER) sizeof( STestFunctionInput ), 0 );
	FATAL_ASSERT( error_code == SQL_SUCCESS );

	error_code = SQLBindParameter( statement_handle, 1, SQL_PARAM_OUTPUT, SQL_C_UBIGINT, SQL_BIGINT, 0, 0, &test_input[ 0 ].AccountID, 0, &test_input[ 0 ].AccountIDLengthIndicator );
	FATAL_ASSERT( error_code == SQL_SUCCESS );

	error_code = SQLBindParameter( statement_handle, 2, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_CHAR, 256, 0, test_input[ 0 ].EmailValue, 256, &test_input[ 0 ].EmailLengthIndicator );
	FATAL_ASSERT( error_code == SQL_SUCCESS );

	error_code = SQLSetStmtAttr( statement_handle, SQL_ATTR_PARAMSET_SIZE, (SQLPOINTER) 2, 0 );
	FATAL_ASSERT( error_code == SQL_SUCCESS );

	std::wstring statement_text( L"{? = call dynamic.get_account_id_by_email(?)}" );
	error_code = SQLExecDirect( statement_handle, (SQLWCHAR *)statement_text.c_str(), SQL_NTS );
	FATAL_ASSERT( error_code == SQL_SUCCESS );

	// functions not allowed to return result sets
/*
	// SNullOutput test_output[ 3 ];
	SQLSMALLINT row_statuses[ 3 ];

	error_code = SQLSetStmtAttr( statement_handle, SQL_ATTR_ROW_BIND_TYPE, (SQLPOINTER) sizeof( SNullOutput ), 0 );
	FATAL_ASSERT( error_code == SQL_SUCCESS );

	error_code = SQLSetStmtAttr( statement_handle, SQL_ATTR_ROW_ARRAY_SIZE, (SQLPOINTER) 3, 0 );
	FATAL_ASSERT( error_code == SQL_SUCCESS );

	error_code = SQLSetStmtAttr( statement_handle, SQL_ATTR_ROW_STATUS_PTR, row_statuses, 0 );
	FATAL_ASSERT( error_code == SQL_SUCCESS );

	int64 rows_fetched = 0;
	error_code = SQLSetStmtAttr( statement_handle, SQL_ATTR_ROWS_FETCHED_PTR, (SQLPOINTER) &rows_fetched, 0 );
	FATAL_ASSERT( error_code == SQL_SUCCESS );

	SQLRETURN ec2 = SQL_SUCCESS;
	while ( ec2 == SQL_SUCCESS )
	{
		while ( error_code == SQL_SUCCESS )
		{
			error_code = SQLFetchScroll( statement_handle, SQL_FETCH_NEXT, 0 );
		}

		if ( error_code == SQL_NO_DATA )
		{
			ec2 = SQLMoreResults( statement_handle );
			error_code = SQL_SUCCESS;
		}
		else
		{
			SQLSMALLINT text_length = 0;
			SQLINTEGER sql_error_code = 0;
			SQLWCHAR error_buffer[ 1024 ] = { 0 };
			SQLWCHAR sql_state[ 6 ] = { 0 };

			error_code = SQLError( env_handle, dbc_handle, statement_handle, sql_state, &sql_error_code, error_buffer, 1024, &text_length );
			while ( error_code == SQL_SUCCESS )
			{
				error_code = SQLError( env_handle, dbc_handle, statement_handle, sql_state, &sql_error_code, error_buffer, 1024, &text_length );
			}

			ec2 = SQL_ERROR;
		}
	}

	if ( ec2 == SQL_NO_DATA_FOUND )
	{
		error_code = SQLEndTran( SQL_HANDLE_DBC, dbc_handle, SQL_COMMIT );
		FATAL_ASSERT( error_code == SQL_SUCCESS );
	}
*/

	error_code = SQLEndTran( SQL_HANDLE_DBC, dbc_handle, SQL_COMMIT );
	FATAL_ASSERT( error_code == SQL_SUCCESS );

	SQLFreeHandle( SQL_HANDLE_STMT, statement_handle );
	SQLDisconnect( dbc_handle );
	SQLFreeHandle( SQL_HANDLE_DBC, dbc_handle );
	SQLFreeHandle( SQL_HANDLE_ENV, env_handle );
}

struct STestInOutInput
{
	SQLUBIGINT AccountCount;
	SQLLEN AccountCountLengthIndicator;
};

struct STestInOutOutput
{
	SQLUBIGINT AccountID;
	SQLLEN AccountIDLengthIndicator;

	SQLCHAR Nickname[ 33 ];
	SQLLEN NicknameLengthIndicator;

	SQLUINTEGER NicknameSequenceID;
	SQLLEN NicknameSequenceIDLengthIndicator;
};

void Test_SQL_Server_Output_Params_Multi_Result_Set( void )
{
	SQLHENV env_handle = 0;
	SQLRETURN error_code = SQLAllocHandle( SQL_HANDLE_ENV, SQL_NULL_HANDLE, &env_handle );
	FATAL_ASSERT( error_code == SQL_SUCCESS );

	error_code = SQLSetEnvAttr( env_handle, SQL_ATTR_ODBC_VERSION, (void*) SQL_OV_ODBC3, 0 );
	FATAL_ASSERT( error_code == SQL_SUCCESS );

	SQLHDBC dbc_handle = 0;
	error_code = SQLAllocHandle( SQL_HANDLE_DBC, env_handle, &dbc_handle );
	FATAL_ASSERT( error_code == SQL_SUCCESS );

	SQLWCHAR output_connection_buffer[ 1024 ] = { 0 };
	SQLSMALLINT output_size = 0;

	error_code = SQLDriverConnect( dbc_handle, 
											 0, 
											 (SQLWCHAR *)L"Driver={SQL Server Native Client 11.0};Server=AZAZELPC\\CCGONLINE;Database=testdb;UID=sa;PWD=CCG0nl1ne;", 
											 SQL_NTS, 
											 output_connection_buffer, 
											 1024, 
											 &output_size, 
											 SQL_DRIVER_COMPLETE );
	FATAL_ASSERT( error_code == SQL_SUCCESS ||error_code == SQL_SUCCESS_WITH_INFO );

	error_code = SQLSetConnectAttr( dbc_handle, SQL_ATTR_AUTOCOMMIT, (SQLPOINTER) SQL_AUTOCOMMIT_OFF, 0 );
	FATAL_ASSERT( error_code == SQL_SUCCESS );

	SQLHSTMT statement_handle = 0;
	error_code = SQLAllocHandle( SQL_HANDLE_STMT, dbc_handle, &statement_handle );
	FATAL_ASSERT( error_code == SQL_SUCCESS );

	STestInOutInput test_input[ 2 ];
	test_input[ 0 ].AccountCount = 0;
	test_input[ 0 ].AccountCountLengthIndicator = 0;

	test_input[ 1 ].AccountCount = 0;
	test_input[ 1 ].AccountCountLengthIndicator = 0;

	error_code = SQLSetStmtAttr( statement_handle, SQL_ATTR_PARAM_BIND_TYPE, (SQLPOINTER) sizeof( STestInOutInput ), 0 );
	FATAL_ASSERT( error_code == SQL_SUCCESS );

	error_code = SQLBindParameter( statement_handle, 1, SQL_PARAM_INPUT_OUTPUT, SQL_C_UBIGINT, SQL_BIGINT, 0, 0, &test_input[ 0 ].AccountCount, 0, &test_input[ 0 ].AccountCountLengthIndicator );
	FATAL_ASSERT( error_code == SQL_SUCCESS );

	STestInOutOutput test_output[ 2 ];
	SQLSMALLINT row_statuses[ 2 ];

	error_code = SQLSetStmtAttr( statement_handle, SQL_ATTR_ROW_BIND_TYPE, (SQLPOINTER) sizeof( STestInOutOutput ), 0 );
	FATAL_ASSERT( error_code == SQL_SUCCESS );

	error_code = SQLSetStmtAttr( statement_handle, SQL_ATTR_ROW_ARRAY_SIZE, (SQLPOINTER) 2, 0 );
	FATAL_ASSERT( error_code == SQL_SUCCESS );

	error_code = SQLSetStmtAttr( statement_handle, SQL_ATTR_ROW_STATUS_PTR, row_statuses, 0 );
	FATAL_ASSERT( error_code == SQL_SUCCESS );

	int64 rows_fetched = 0;
	error_code = SQLSetStmtAttr( statement_handle, SQL_ATTR_ROWS_FETCHED_PTR, (SQLPOINTER) &rows_fetched, 0 );
	FATAL_ASSERT( error_code == SQL_SUCCESS );

	error_code = SQLBindCol( statement_handle, 1, SQL_C_UBIGINT, &test_output[ 0 ].AccountID, 0, &test_output[ 0 ].AccountIDLengthIndicator );
	FATAL_ASSERT( error_code == SQL_SUCCESS );

	error_code = SQLBindCol( statement_handle, 2, SQL_C_CHAR, test_output[ 0 ].Nickname, 33, &test_output[ 0 ].NicknameLengthIndicator );
	FATAL_ASSERT( error_code == SQL_SUCCESS );

	error_code = SQLBindCol( statement_handle, 3, SQL_C_ULONG, &test_output[ 0 ].NicknameSequenceID, 0, &test_output[ 0 ].NicknameSequenceIDLengthIndicator );
	FATAL_ASSERT( error_code == SQL_SUCCESS );

	error_code = SQLSetStmtAttr( statement_handle, SQL_ATTR_PARAMSET_SIZE, (SQLPOINTER) 2, 0 );
	FATAL_ASSERT( error_code == SQL_SUCCESS );

	std::wstring statement_text( L"{call dynamic.get_all_accounts_with_in_out(?)}" );
	error_code = SQLExecDirect( statement_handle, (SQLWCHAR *)statement_text.c_str(), SQL_NTS );
	FATAL_ASSERT( error_code == SQL_SUCCESS );

	SQLRETURN ec2 = SQL_SUCCESS;
	while ( ec2 == SQL_SUCCESS )
	{
		while ( error_code == SQL_SUCCESS )
		{
			error_code = SQLFetchScroll( statement_handle, SQL_FETCH_NEXT, 0 );
		}

		if ( error_code == SQL_NO_DATA )
		{
			ec2 = SQLMoreResults( statement_handle );
			error_code = SQL_SUCCESS;
		}
		else
		{
			SQLSMALLINT text_length = 0;
			SQLINTEGER sql_error_code = 0;
			SQLWCHAR error_buffer[ 1024 ] = { 0 };
			SQLWCHAR sql_state[ 6 ] = { 0 };

			error_code = SQLError( env_handle, dbc_handle, statement_handle, sql_state, &sql_error_code, error_buffer, 1024, &text_length );
			while ( error_code == SQL_SUCCESS )
			{
				error_code = SQLError( env_handle, dbc_handle, statement_handle, sql_state, &sql_error_code, error_buffer, 1024, &text_length );
			}

			ec2 = SQL_ERROR;
		}
	}

	if ( ec2 == SQL_NO_DATA_FOUND )
	{
		error_code = SQLEndTran( SQL_HANDLE_DBC, dbc_handle, SQL_COMMIT );
		FATAL_ASSERT( error_code == SQL_SUCCESS );
	}

	SQLFreeHandle( SQL_HANDLE_STMT, statement_handle );
	SQLDisconnect( dbc_handle );
	SQLFreeHandle( SQL_HANDLE_DBC, dbc_handle );
	SQLFreeHandle( SQL_HANDLE_ENV, env_handle );
}

struct STestAccountGetOutput
{
	SQLUBIGINT AccountID;
	SQLLEN AccountIDLengthIndicator;

	SQLCHAR AccountEmail[ 256 ];
	SQLLEN AccountEmailLengthIndicator;

	SQLCHAR Nickname[ 33 ];
	SQLLEN NicknameLengthIndicator;

	SQLUINTEGER NicknameSequenceID;
	SQLLEN NicknameSequenceIDLengthIndicator;
};

void Test_SQL_Server_Multirow_Result_Set( void )
{
	SQLHENV env_handle = 0;
	SQLRETURN error_code = SQLAllocHandle( SQL_HANDLE_ENV, SQL_NULL_HANDLE, &env_handle );
	FATAL_ASSERT( error_code == SQL_SUCCESS );

	error_code = SQLSetEnvAttr( env_handle, SQL_ATTR_ODBC_VERSION, (void*) SQL_OV_ODBC3, 0 );
	FATAL_ASSERT( error_code == SQL_SUCCESS );

	SQLHDBC dbc_handle = 0;
	error_code = SQLAllocHandle( SQL_HANDLE_DBC, env_handle, &dbc_handle );
	FATAL_ASSERT( error_code == SQL_SUCCESS );

	SQLWCHAR output_connection_buffer[ 1024 ] = { 0 };
	SQLSMALLINT output_size = 0;

	error_code = SQLDriverConnect( dbc_handle, 
											 0, 
											 (SQLWCHAR *)L"Driver={SQL Server Native Client 11.0};Server=AZAZELPC\\CCGONLINE;Database=testdb;UID=sa;PWD=CCG0nl1ne;", 
											 SQL_NTS, 
											 output_connection_buffer, 
											 1024, 
											 &output_size, 
											 SQL_DRIVER_COMPLETE );
	FATAL_ASSERT( error_code == SQL_SUCCESS ||error_code == SQL_SUCCESS_WITH_INFO );

	error_code = SQLSetConnectAttr( dbc_handle, SQL_ATTR_AUTOCOMMIT, (SQLPOINTER) SQL_AUTOCOMMIT_OFF, 0 );
	FATAL_ASSERT( error_code == SQL_SUCCESS );

	SQLHSTMT statement_handle = 0;
	error_code = SQLAllocHandle( SQL_HANDLE_STMT, dbc_handle, &statement_handle );
	FATAL_ASSERT( error_code == SQL_SUCCESS );

	STestInOutInput test_input[ 2 ];
	test_input[ 0 ].AccountCount = 0;
	test_input[ 0 ].AccountCountLengthIndicator = 0;

	error_code = SQLSetStmtAttr( statement_handle, SQL_ATTR_PARAM_BIND_TYPE, (SQLPOINTER) sizeof( STestInOutInput ), 0 );
	FATAL_ASSERT( error_code == SQL_SUCCESS );

	error_code = SQLBindParameter( statement_handle, 1, SQL_PARAM_INPUT, SQL_C_UBIGINT, SQL_BIGINT, 0, 0, &test_input[ 0 ].AccountCount, 0, &test_input[ 0 ].AccountCountLengthIndicator );
	FATAL_ASSERT( error_code == SQL_SUCCESS );

	STestAccountGetOutput test_output[ 2 ];
	SQLSMALLINT row_statuses[ 2 ];

	error_code = SQLSetStmtAttr( statement_handle, SQL_ATTR_ROW_BIND_TYPE, (SQLPOINTER) sizeof( STestAccountGetOutput ), 0 );
	FATAL_ASSERT( error_code == SQL_SUCCESS );

	error_code = SQLSetStmtAttr( statement_handle, SQL_ATTR_ROW_ARRAY_SIZE, (SQLPOINTER) 1, 0 );
	FATAL_ASSERT( error_code == SQL_SUCCESS );

	error_code = SQLSetStmtAttr( statement_handle, SQL_ATTR_ROW_STATUS_PTR, row_statuses, 0 );
	FATAL_ASSERT( error_code == SQL_SUCCESS );

	int64 rows_fetched = 0;
	error_code = SQLSetStmtAttr( statement_handle, SQL_ATTR_ROWS_FETCHED_PTR, (SQLPOINTER) &rows_fetched, 0 );
	FATAL_ASSERT( error_code == SQL_SUCCESS );

	error_code = SQLBindCol( statement_handle, 1, SQL_C_UBIGINT, &test_output[ 0 ].AccountID, 0, &test_output[ 0 ].AccountIDLengthIndicator );
	FATAL_ASSERT( error_code == SQL_SUCCESS );

	error_code = SQLBindCol( statement_handle, 2, SQL_C_CHAR, test_output[ 0 ].AccountEmail, 256, &test_output[ 0 ].AccountEmailLengthIndicator );
	FATAL_ASSERT( error_code == SQL_SUCCESS );

	error_code = SQLBindCol( statement_handle, 3, SQL_C_CHAR, test_output[ 0 ].Nickname, 33, &test_output[ 0 ].NicknameLengthIndicator );
	FATAL_ASSERT( error_code == SQL_SUCCESS );

	error_code = SQLBindCol( statement_handle, 4, SQL_C_ULONG, &test_output[ 0 ].NicknameSequenceID, 0, &test_output[ 0 ].NicknameSequenceIDLengthIndicator );
	FATAL_ASSERT( error_code == SQL_SUCCESS );

	error_code = SQLSetStmtAttr( statement_handle, SQL_ATTR_PARAMSET_SIZE, (SQLPOINTER) 1, 0 );
	FATAL_ASSERT( error_code == SQL_SUCCESS );

	std::wstring statement_text( L"{call dynamic.get_all_accounts(?)}" );
	error_code = SQLExecDirect( statement_handle, (SQLWCHAR *)statement_text.c_str(), SQL_NTS );
	FATAL_ASSERT( error_code == SQL_SUCCESS );

	error_code = SQLSetStmtAttr( statement_handle, SQL_ATTR_ROW_ARRAY_SIZE, (SQLPOINTER) 2, 0 );
	FATAL_ASSERT( error_code == SQL_SUCCESS );

	SQLRETURN ec2 = SQL_SUCCESS;
	while ( ec2 == SQL_SUCCESS )
	{
		while ( error_code == SQL_SUCCESS )
		{
			error_code = SQLFetchScroll( statement_handle, SQL_FETCH_NEXT, 0 );
		}

		if ( error_code == SQL_NO_DATA )
		{
			ec2 = SQLMoreResults( statement_handle );
			error_code = SQL_SUCCESS;
		}
		else
		{
			SQLSMALLINT text_length = 0;
			SQLINTEGER sql_error_code = 0;
			SQLWCHAR error_buffer[ 1024 ] = { 0 };
			SQLWCHAR sql_state[ 6 ] = { 0 };

			error_code = SQLError( env_handle, dbc_handle, statement_handle, sql_state, &sql_error_code, error_buffer, 1024, &text_length );
			while ( error_code == SQL_SUCCESS )
			{
				error_code = SQLError( env_handle, dbc_handle, statement_handle, sql_state, &sql_error_code, error_buffer, 1024, &text_length );
			}

			ec2 = SQL_ERROR;
		}
	}

	if ( ec2 == SQL_NO_DATA_FOUND )
	{
		error_code = SQLEndTran( SQL_HANDLE_DBC, dbc_handle, SQL_COMMIT );
		FATAL_ASSERT( error_code == SQL_SUCCESS );
	}

	SQLFreeHandle( SQL_HANDLE_STMT, statement_handle );
	SQLDisconnect( dbc_handle );
	SQLFreeHandle( SQL_HANDLE_DBC, dbc_handle );
	SQLFreeHandle( SQL_HANDLE_ENV, env_handle );
}

#endif 

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

class CAddAccountInputParams : public CODBCVariableSet
{
	public:

		typedef CODBCVariableSet BASECLASS;

		CAddAccountInputParams( void ) :
			BASECLASS(),
			AccountEmail(),
			Nickname(),
			PasswordHash()
		{}

		CAddAccountInputParams( const std::string &account_email, const std::string &nickname, const std::string &password_hash ) :
			BASECLASS(),
			AccountEmail( account_email ),
			Nickname( nickname ),
			PasswordHash( password_hash )
		{}

		virtual ~CAddAccountInputParams() {}

		virtual void Get_Variables( std::vector< IDatabaseVariable * > &variables )
		{
			variables.push_back( &AccountEmail );
			variables.push_back( &Nickname );
			variables.push_back( &PasswordHash );
		}

		DBString< 255 > AccountEmail;
		DBString< 32 > Nickname;
		DBString< 32 > PasswordHash;
};

bool Handle_Add_Account( const CSlashCommandInstance &instance, std::wstring & /*error_msg*/ )
{
	IDatabaseConnection *connection = CODBCFactory::Get_Environment()->Add_Connection( L"Driver={SQL Server Native Client 11.0};Server=AZAZELPC\\CCGONLINE;Database=testdb;UID=testserver;PWD=TEST5erver#;", false );
	FATAL_ASSERT( connection != nullptr );

	IDatabaseStatement *statement = connection->Allocate_Statement( L"{call dynamic.add_account(?,?,?)}" );
	FATAL_ASSERT( statement != nullptr );

	std::string email;
	instance.Get_Param( 0, email );

	std::string nickname;
	instance.Get_Param( 1, nickname );

	uint32 add_count = 0;
	instance.Get_Param( 2, add_count );
	FATAL_ASSERT( add_count > 0 );

	CAddAccountInputParams *params_array = new CAddAccountInputParams[ add_count ];

/*
	for ( uint32 i = 0; i < add_count; i++ )
	{
		char suffix[ 2 ];
		suffix[ 1 ] = 0;
		suffix[ 0 ] = 'A' + (char) i;

		params_array[ i ] = CAddAccountInputParams( email + suffix, nickname, "00001111222233334444555566667777" );
	}
*/

	for ( uint32 i = 0; i < add_count; i++ )
	{
		params_array[ i ] = CAddAccountInputParams( email, nickname, "00001111222233334444555566667777" );
	}

	statement->Bind_Input( params_array, sizeof( CAddAccountInputParams ) );

	CEmptyVariableSet result_set;
	statement->Bind_Output( &result_set, sizeof( CEmptyVariableSet ), 1 );
	
	statement->Execute( add_count );

	int64 rows_fetched = 0;
	EFetchResultsStatusType fetch_status = FRST_ONGOING;
	while ( fetch_status != FRST_ERROR && fetch_status != FRST_FINISHED_ALL )
	{
		fetch_status = statement->Fetch_Results( rows_fetched );
	}

	connection->End_Transaction( true );

	connection->Release_Statement( statement );
	CODBCFactory::Get_Environment()->Shutdown_Connection( connection->Get_ID() );
	delete connection;

	delete []params_array;

	return true;
}

class CFetchAccountInputParams : public CODBCVariableSet
{
	public:

		typedef CODBCVariableSet BASECLASS;

		CFetchAccountInputParams( void ) :
			BASECLASS(),
			AccountEmail()
		{}

		CFetchAccountInputParams( const std::string &account_email ) :
			BASECLASS(),
			AccountEmail( account_email )
		{}

		virtual ~CFetchAccountInputParams() {}

		virtual void Get_Variables( std::vector< IDatabaseVariable * > &variables )
		{
			variables.push_back( &AccountEmail );
		}

		DBString< 255 > AccountEmail;
};

class CFetchAccountResultSet : public CODBCVariableSet
{
	public:

		typedef CODBCVariableSet BASECLASS;

		CFetchAccountResultSet( void ) :
			BASECLASS(),
			AccountID(),
			Nickname(),
			NicknameSequenceID()
		{}

		virtual ~CFetchAccountResultSet() {}

		virtual void Get_Variables( std::vector< IDatabaseVariable * > &variables )
		{
			variables.push_back( &AccountID );
			variables.push_back( &Nickname );
			variables.push_back( &NicknameSequenceID );
		}

		DBUInt64In AccountID;
		DBString< 32 > Nickname;
		DBUInt32In NicknameSequenceID;
};

bool Handle_Fetch_Account( const CSlashCommandInstance &instance, std::wstring & /*error_msg*/ )
{
	IDatabaseConnection *connection = CODBCFactory::Get_Environment()->Add_Connection( L"Driver={SQL Server Native Client 11.0};Server=AZAZELPC\\CCGONLINE;Database=testdb;UID=testserver;PWD=TEST5erver#;", false );
	FATAL_ASSERT( connection != nullptr );

	IDatabaseStatement *statement = connection->Allocate_Statement( L"{call dynamic.get_account_by_email(?)}" );
	FATAL_ASSERT( statement != nullptr );

	std::string email;
	instance.Get_Param( 0, email );

	uint32 fetch_count = 0;
	instance.Get_Param( 1, fetch_count );
	FATAL_ASSERT( fetch_count > 0 );

	CFetchAccountInputParams *params_array = new CFetchAccountInputParams[ fetch_count ];

	for ( uint32 i = 0; i < fetch_count; i++ )
	{
		params_array[ i ] = CFetchAccountInputParams( email );
	}

	statement->Bind_Input( params_array, sizeof( CAddAccountInputParams ) );

	CFetchAccountResultSet result_set;
	statement->Bind_Output( &result_set, sizeof( CFetchAccountResultSet ), 1 );
	
	statement->Execute( fetch_count );

	int64 rows_fetched = 0;
	EFetchResultsStatusType fetch_status = FRST_ONGOING;
	while ( fetch_status != FRST_ERROR && fetch_status != FRST_FINISHED_ALL )
	{
		fetch_status = statement->Fetch_Results( rows_fetched );
	}

	connection->End_Transaction( true );

	connection->Release_Statement( statement );
	CODBCFactory::Get_Environment()->Shutdown_Connection( connection->Get_ID() );
	delete connection;

	delete []params_array;

	return true;
}

class CAddAccountDatabaseTask : public TDatabaseProcedureCall< CAddAccountInputParams, 3, CEmptyVariableSet, 1 >
{
	public:

		CAddAccountDatabaseTask( const std::string &account_email, const std::string &nickname, const std::string &password_hash ) :
			AccountEmail( account_email ),
			Nickname( nickname ),
			PasswordHash( password_hash )
		{}

		virtual ~CAddAccountDatabaseTask() {}

	protected:

		virtual void Initialize_Parameters( IDatabaseVariableSet *input_parameters )
		{
			CAddAccountInputParams *input_params = static_cast< CAddAccountInputParams * >( input_parameters );
			*input_params = CAddAccountInputParams( AccountEmail, Nickname, PasswordHash );
		}

		virtual void On_Fetch_Results( IDatabaseVariableSet * /*result_set*/, int64 /*rows_fetched*/ ) {}		
		virtual void On_Fetch_Results_Finished( IDatabaseVariableSet * /*input_parameters*/ ) {}

		virtual void On_Rollback( void ) {}
		virtual void On_Task_Success( void ) {}				
		virtual void On_Task_Failure( void ) {}

		virtual const wchar_t *Get_Database_Object_Name( void ) const { return L"dynamic.add_account"; }

	private:

		std::string AccountEmail;
		std::string Nickname;
		std::string PasswordHash;
};

bool Handle_Add_Account_Batch( const CSlashCommandInstance &instance, std::wstring & /*error_msg*/ )
{
	std::string email;
	instance.Get_Param( 0, email );

	std::string nickname;
	instance.Get_Param( 1, nickname );

	uint32 add_count = 0;
	instance.Get_Param( 2, add_count );
	FATAL_ASSERT( add_count > 0 );

	std::vector< IDatabaseTask * > tasks;
	TDatabaseTaskBatch< CAddAccountDatabaseTask > add_account_batch;

	for ( uint32 i = 0; i < add_count; ++i )
	{
		char buffer[ 10 ];
		_itoa_s( i, buffer, 10 );
		IDatabaseTask *task = new CAddAccountDatabaseTask( email + std::string( buffer ), nickname, "00001111222233334444555566667777" );
		add_account_batch.Add_Task( task );
		tasks.push_back( task ); 
	}

	IDatabaseConnection *connection = CODBCFactory::Get_Environment()->Add_Connection( L"Driver={SQL Server Native Client 11.0};Server=AZAZELPC\\CCGONLINE;Database=testdb;UID=testserver;PWD=TEST5erver#;", false );
	FATAL_ASSERT( connection != nullptr );

	DBTaskBaseListType successful_tasks, failed_tasks;
	add_account_batch.Execute_Tasks( connection, successful_tasks, failed_tasks );

	CODBCFactory::Get_Environment()->Shutdown_Connection( connection->Get_ID() );
	delete connection;
	
	for ( uint32 i = 0; i < add_count; ++i )
	{
		delete tasks[ i ];
	}

	return true;
}

bool Handle_Fetch_Account_Batch( const CSlashCommandInstance & /*instance*/, std::wstring & /*error_msg*/ )
{
	return true;
}

int main( int /*argc*/, wchar_t* /*argv*/[] )
{
	NAuthServer::Initialize();

	Test_SQL_Server_Multirow_Result_Set();

	CSlashCommandManager::Initialize();
	CSlashCommandManager::Load_Command_File( "Data/XML/SlashCommandODBCTests.xml" );
	CSlashCommandManager::Register_Command_Handler( L"AddAccount", Handle_Add_Account );
	CSlashCommandManager::Register_Command_Handler( L"FetchAccount", Handle_Fetch_Account );
	CSlashCommandManager::Register_Command_Handler( L"BatchAddAccount", Handle_Add_Account_Batch );
	CSlashCommandManager::Register_Command_Handler( L"BatchFetchAccount", Handle_Fetch_Account_Batch );

	CODBCFactory::Create_Environment();

	bool done = false;
	while ( !done )
	{
		wchar_t command_line[ 256 ];
		std::wcin.getline( command_line, sizeof( command_line ) );

		if ( _wcsicmp( command_line, L"quit" ) == 0 )
		{
			done = true;
		}
		else
		{
			std::wstring error_msg;
			CSlashCommandInstance command_instance;
			if ( CSlashCommandManager::Parse_Command( command_line, command_instance, error_msg ) )
			{
				std::wcout << L"Handling command\n";
				if ( !CSlashCommandManager::Handle_Command( command_instance, error_msg ) )
				{
					std::wcout << L"There was an error executing the command: " << error_msg << L"\n";
				}
				else
				{
					std::wcout << L"Success!\n";
				}
			}
			else
			{
				std::wcout << L"Parse error:" << error_msg << L"\n";
			}	
		}
	}

	CODBCFactory::Destroy_Environment();

	CSlashCommandManager::Shutdown();

	//Test_SQL_Server_Output_Params_Multi_Result_Set();
	//Test_SQL_Server_Function_No_Results();
	//Test_SQL_Server_Multi_Result_Set();
	//Test_SQL_Server_Single_Result_Set();
	//Test_SQL_Server_Input_Only();
	//Test_Sql_Server_Connection();
	//Test_Oracle_Procedure_Call_Multi_Output();
	//Test_Oracle_Procedure_Call_Get_Output();
	//Test_Oracle_Procedure_Call_Input_Only();
	// Test_Oracle_Connection();
	// Sql_Stuff2();
	//SQL_Stuff();
	//Exception_Test_1();

	NAuthServer::Shutdown();

	return 0;
}

