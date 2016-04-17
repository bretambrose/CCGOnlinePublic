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

#pragma once

#include <IPDatabase/IPDatabase.h>

#include <IPCore/Memory/Stl/String.h>
#include <IPCore/Memory/Stl/Vector.h>

// TODO: platformize
#include <IPCore/System/WindowsWrapper.h>
#include <sql.h>

enum DBErrorStateType;

namespace IP
{
namespace Db
{

IPDATABASE_API struct SODBCError
{
	SODBCError( void );
	SODBCError( const SODBCError &rhs );
	SODBCError( int32_t sql_error_code, char *sql_state, const IP::String &error_description );

	int32_t SQLErrorCode;

	IP::String SQLState;
	IP::String ErrorDescription;
};

IPDATABASE_API class CODBCObjectBase
{
	public:

		CODBCObjectBase( SQLHENV environment_handle, SQLHDBC connection_handle, SQLHSTMT statement_handle );
		virtual ~CODBCObjectBase();

	protected:

		void Invalidate_Handles( void );

		bool Refresh_Error_Status( SQLRETURN error_code );	

		DBErrorStateType Get_Error_State_Base( void ) const { return ErrorState; }
		void Set_Error_State_Base( DBErrorStateType error_state ) { ErrorState = error_state; }
		void Push_User_Error( DBErrorStateType error_state, const IP::String &error_description );

		int32_t Get_Bad_Row_Number_Base( void ) const { return BadRowNumber; }
		void Set_Bad_Row_Number_Base( int32_t bad_row_number ) { BadRowNumber = bad_row_number; }

		const IP::Vector< SODBCError > &Get_Errors( void ) const { return Errors; }

		void Log_Error_State_Base( void ) const;
		
		SQLHENV EnvironmentHandle;
		SQLHDBC ConnectionHandle;
		SQLHSTMT StatementHandle;

	private:

		DBErrorStateType ErrorState;
		int32_t BadRowNumber;

		IP::Vector< SODBCError > Errors;
};

} // namespace Db
} // namespace IP