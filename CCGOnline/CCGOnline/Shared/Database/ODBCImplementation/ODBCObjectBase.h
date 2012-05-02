/**********************************************************************************************************************

	ODBCObjectBase.h
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

#ifndef ODBC_OBJECT_BASE_H
#define ODBC_OBJECT_BASE_H

#include <Windows.h>
#include <sql.h>

struct SODBCError
{
	SODBCError( void );
	SODBCError( const SODBCError &rhs );
	SODBCError( int32 sql_error_code, wchar_t sql_state[ 6 ], const std::wstring &error_description );

	int32 SQLErrorCode;

	std::wstring SQLState;
	std::wstring ErrorDescription;
};

class CODBCObjectBase
{
	public:

		CODBCObjectBase( SQLHENV environment_handle, SQLHDBC connection_handle, SQLHSTMT statement_handle );
		virtual ~CODBCObjectBase();

	protected:

		void Invalidate_Handles( void );

		void Clear_Error_List( void );
		void Rebuild_Error_List( void );	

		const std::vector< SODBCError > &Get_Errors( void ) const { return Errors; }
		
		SQLHENV EnvironmentHandle;
		SQLHDBC ConnectionHandle;
		SQLHSTMT StatementHandle;

	private:

		std::vector< SODBCError > Errors;
};

#endif // ODBC_OBJECT_BASE_H