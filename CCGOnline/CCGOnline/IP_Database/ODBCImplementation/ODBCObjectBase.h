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

#include "WindowsWrapper.h"
#include <sql.h>

enum DBErrorStateType;

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

		bool Refresh_Error_Status( SQLRETURN error_code );	

		DBErrorStateType Get_Error_State_Base( void ) const { return ErrorState; }
		void Set_Error_State_Base( DBErrorStateType error_state ) { ErrorState = error_state; }
		void Push_User_Error( DBErrorStateType error_state, const std::wstring &error_description );

		int32 Get_Bad_Row_Number_Base( void ) const { return BadRowNumber; }
		void Set_Bad_Row_Number_Base( int32 bad_row_number ) { BadRowNumber = bad_row_number; }

		const std::vector< SODBCError > &Get_Errors( void ) const { return Errors; }

		void Log_Error_State_Base( void ) const;
		
		SQLHENV EnvironmentHandle;
		SQLHDBC ConnectionHandle;
		SQLHSTMT StatementHandle;

	private:

		DBErrorStateType ErrorState;
		int32 BadRowNumber;

		std::vector< SODBCError > Errors;
};

#endif // ODBC_OBJECT_BASE_H