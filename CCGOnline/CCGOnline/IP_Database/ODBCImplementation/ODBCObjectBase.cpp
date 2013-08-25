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
#include "DatabaseTypes.h"
#include "Logging/LogInterface.h"
#include "EnumConversion.h"

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
	ErrorState( DBEST_SUCCESS ),
	BadRowNumber( -1 ),
	Errors()
{
}

CODBCObjectBase::~CODBCObjectBase()
{
}

bool CODBCObjectBase::Refresh_Error_Status( SQLRETURN error_code )
{
	FATAL_ASSERT( EnvironmentHandle != 0 );

	Errors.clear();
	BadRowNumber = -1;

	if ( error_code == SQL_SUCCESS )
	{
		ErrorState = DBEST_SUCCESS;
		return true;
	}
	else if ( error_code == SQL_INVALID_HANDLE )
	{
		ErrorState = DBEST_FATAL_ERROR;
		return true;
	}

	SQLSMALLINT handle_type = SQL_HANDLE_ENV;
	SQLHANDLE handle = EnvironmentHandle;
	if ( StatementHandle != 0 )
	{
		handle_type = SQL_HANDLE_STMT;
		handle = StatementHandle;
	}
	else if ( ConnectionHandle != 0 )
	{
		handle_type = SQL_HANDLE_DBC;
		handle = ConnectionHandle;
	}

	SQLINTEGER record_count = 0;
	SQLRETURN ec = SQLGetDiagField( handle_type, handle, 0, SQL_DIAG_NUMBER, &record_count, SQL_IS_INTEGER, NULL );
	FATAL_ASSERT( ec == SQL_SUCCESS );

	SQLWCHAR sql_state[ 6 ] = { 0 };
	SQLINTEGER sql_error_code = 0;
	SQLSMALLINT text_length = 0;
	SQLWCHAR error_buffer[ 1024 ] = { 0 };

	for ( SQLSMALLINT i = 1; i <= record_count; ++i )
	{
		ec = SQLGetDiagRec( handle_type, handle, i, sql_state, &sql_error_code, error_buffer, sizeof( error_buffer ) / sizeof( SQLWCHAR ), &text_length );
		FATAL_ASSERT( ec == SQL_SUCCESS );

		Errors.push_back( SODBCError( sql_error_code, sql_state, std::wstring( error_buffer ) ) );

		if ( handle_type == SQL_HANDLE_STMT && BadRowNumber == -1 )
		{
			SQLLEN bad_row = 0;
			ec = SQLGetDiagField( handle_type, handle, i, SQL_DIAG_ROW_NUMBER, &bad_row, SQL_IS_INTEGER, NULL );
			FATAL_ASSERT( ec == SQL_SUCCESS );

			if ( bad_row >= 1 )
			{
				BadRowNumber = static_cast< int32 >( bad_row - 1 );
			}
		}
	}

	return false;
}

void CODBCObjectBase::Invalidate_Handles( void )
{
	EnvironmentHandle = 0;
	ConnectionHandle = 0;
	StatementHandle = 0;
}

void CODBCObjectBase::Push_User_Error( DBErrorStateType error_state, const std::wstring &error_description )
{
	FATAL_ASSERT( error_state != DBEST_SUCCESS );

	ErrorState = error_state;

	Errors.clear();
	Errors.push_back( SODBCError( -1, L"*USER*", error_description ) );
}

void CODBCObjectBase::Log_Error_State_Base( void ) const
{
	std::string state_string;
	CEnumConverter::Convert< DBErrorStateType >( ErrorState, state_string );
	LOG( LL_LOW, "\tState: " << state_string.c_str() );
	if ( Errors.size() > 0 )
	{
		LOG( LL_LOW, "\tODBC Error Records:" );
		for ( uint32 i = 0; i < Errors.size(); ++i )
		{
			const SODBCError &error_record = Errors[ i ]; 
			WLOG( LL_LOW, L"\t\t" << i << L" - EC: " << error_record.SQLErrorCode << L", SQLState: " << error_record.SQLState << L", Desc: " << error_record.ErrorDescription );
		}
	}
	else
	{
		LOG( LL_LOW, "\tNo ODBC Error records found." );
	}
}

