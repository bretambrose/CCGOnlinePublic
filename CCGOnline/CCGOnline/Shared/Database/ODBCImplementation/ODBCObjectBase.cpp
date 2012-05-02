/**********************************************************************************************************************

	ODBCObjectBase.cpp
		A component that implements a base class for all ODBC-based database object types.  Gathers and tracks
		error information, keeps handles.

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

#include "ODBCObjectBase.h"
#include <sqlucode.h>
#include "Database/DatabaseTypes.h"

SODBCError::SODBCError( void ) :
	SQLErrorCode( SQL_SUCCESS ),
	SQLState( L"" ),
	ErrorDescription( L"" )
{
}

SODBCError::SODBCError( const SODBCError &rhs ) :
	SQLErrorCode( rhs.SQLErrorCode ),
	SQLState( rhs.SQLState ),
	ErrorDescription( rhs.ErrorDescription )
{
}

SODBCError::SODBCError( int32 sql_error_code, wchar_t sql_state[ 6 ], const std::wstring &error_description ) :
	SQLErrorCode( sql_error_code ),
	SQLState( sql_state, sql_state + 6 ),	// sketchy
	ErrorDescription( error_description )
{
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

CODBCObjectBase::CODBCObjectBase( SQLHENV environment_handle, SQLHDBC connection_handle, SQLHSTMT statement_handle ) :
	EnvironmentHandle( environment_handle ),
	ConnectionHandle( connection_handle ),
	StatementHandle( statement_handle ),
	Errors()
{
}

CODBCObjectBase::~CODBCObjectBase()
{
}

void CODBCObjectBase::Clear_Error_List( void )
{
	Errors.clear();
}

void CODBCObjectBase::Rebuild_Error_List( void )
{
	FATAL_ASSERT( EnvironmentHandle != 0 );
	Errors.clear();

	SQLSMALLINT text_length = 0;
	SQLINTEGER sql_error_code = 0;
	SQLWCHAR error_buffer[ 1024 ] = { 0 };
	SQLWCHAR sql_state[ 6 ] = { 0 };

	SQLRETURN error_code = SQLError( EnvironmentHandle, 
												( ConnectionHandle != 0 ) ? ConnectionHandle : SQL_NULL_HDBC, 
												( StatementHandle != 0 ) ? StatementHandle : SQL_NULL_HSTMT, 
												sql_state, 
												&sql_error_code, 
												error_buffer, 
												sizeof( error_buffer ), 
												&text_length );

	Errors.push_back( SODBCError( sql_error_code, sql_state, std::wstring( error_buffer, error_buffer + text_length ) ) );

	while ( error_code == SQL_SUCCESS )
	{
		error_code = SQLError( EnvironmentHandle, 
									  ( ConnectionHandle != 0 ) ? ConnectionHandle : SQL_NULL_HDBC, 
									  ( StatementHandle != 0 ) ? StatementHandle : SQL_NULL_HSTMT, 
									  sql_state, 
									  &sql_error_code, 
									  error_buffer, 
									  sizeof( error_buffer ), 
									  &text_length );

		Errors.push_back( SODBCError( sql_error_code, sql_state, std::wstring( error_buffer, error_buffer + text_length ) ) );
	}
}

void CODBCObjectBase::Invalidate_Handles( void )
{
	EnvironmentHandle = 0;
	ConnectionHandle = 0;
	StatementHandle = 0;
}


