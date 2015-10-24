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

#include "IPDatabase/Interfaces/DatabaseEnvironmentInterface.h"
#include "ODBCObjectBase.h"

namespace IP
{
namespace Db
{

enum class ODBCEnvironmentStateType;
enum class ODBCEnvironmentOperationType;

class CODBCEnvironment : public CODBCObjectBase, public IDatabaseEnvironment
{
	public:

		CODBCEnvironment( SQLHENV environment_handle );
		virtual ~CODBCEnvironment();

		virtual void Initialize( void );
		virtual void Shutdown( void );
		virtual void Shutdown_Connection( DBConnectionIDType connection_id );
		virtual void Shutdown_Connection( IDatabaseConnection *connection );

		virtual IDatabaseConnection *Add_Connection( const std::wstring &connection_string, bool cache_statements );
		
		virtual DBErrorStateType Get_Error_State( void ) const { return Get_Error_State_Base(); }

	private:
		
		bool Was_Last_ODBC_Operation_Successful( void ) const;

		void Update_Error_Status( ODBCEnvironmentOperationType operation_type, SQLRETURN error_code );

		ODBCEnvironmentStateType State;

		std::unordered_map< DBConnectionIDType, IDatabaseConnection * > Connections;
	
		DBConnectionIDType NextConnectionID;

};

} // namespace Db
} // namespace IP