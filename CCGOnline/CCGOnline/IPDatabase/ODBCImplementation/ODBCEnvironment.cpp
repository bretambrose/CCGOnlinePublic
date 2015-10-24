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

#include "stdafx.h"

#include "ODBCEnvironment.h"

#include "ODBCConnection.h"
#include "IPDatabase/DatabaseTypes.h"
#include <sqlext.h>

namespace IP
{
namespace Db
{

enum class ODBCEnvironmentStateType
{
	UNINITIALIZED,
	INITIALIZED,
	SHUTDOWN,

	FATAL_ERROR
};

enum class ODBCEnvironmentOperationType
{
	SET_VERSION,
	ALLOCATE_CONNECTION_HANDLE
};

CODBCEnvironment::CODBCEnvironment( SQLHENV environment_handle ) :
	CODBCObjectBase( environment_handle, 0, 0 ),
	State( ODBCEnvironmentStateType::UNINITIALIZED ),
	Connections(),
	NextConnectionID( static_cast< DBConnectionIDType >( 1 ) )
{
}

CODBCEnvironment::~CODBCEnvironment()
{
	Shutdown();
}

void CODBCEnvironment::Initialize( void )
{
	FATAL_ASSERT( State == ODBCEnvironmentStateType::UNINITIALIZED );

	SQLRETURN error_code = SQLSetEnvAttr( EnvironmentHandle, SQL_ATTR_ODBC_VERSION, (void*) SQL_OV_ODBC3, 0 );
	Update_Error_Status( ODBCEnvironmentOperationType::SET_VERSION, error_code );
	if ( !Was_Last_ODBC_Operation_Successful() )
	{
		State = ODBCEnvironmentStateType::FATAL_ERROR;
		return;
	}

	State = ODBCEnvironmentStateType::INITIALIZED;
}

void CODBCEnvironment::Shutdown( void )
{
	if ( State != ODBCEnvironmentStateType::UNINITIALIZED && State != ODBCEnvironmentStateType::SHUTDOWN )
	{
		for ( auto iter = Connections.begin(); iter != Connections.end(); ++iter )
		{
			iter->second->Shutdown();
			delete iter->second;
		}

		Connections.clear();

		SQLFreeHandle( SQL_HANDLE_ENV, EnvironmentHandle );

		Invalidate_Handles();
	}

	State = ODBCEnvironmentStateType::SHUTDOWN;
}

void CODBCEnvironment::Shutdown_Connection( DBConnectionIDType connection_id )
{
	auto iter = Connections.find( connection_id );
	FATAL_ASSERT( iter != Connections.end() );

	iter->second->Shutdown();
	Connections.erase( iter );
}

void CODBCEnvironment::Shutdown_Connection( IDatabaseConnection *connection )
{
	FATAL_ASSERT( connection != nullptr );
	Shutdown_Connection( connection->Get_ID() );
}

IDatabaseConnection *CODBCEnvironment::Add_Connection( const std::wstring &connection_string, bool cache_statements )
{
	if ( State != ODBCEnvironmentStateType::INITIALIZED )
	{
		return nullptr;
	}

	SQLHDBC dbc_handle = 0;
	SQLRETURN error_code = SQLAllocHandle( SQL_HANDLE_DBC, EnvironmentHandle, &dbc_handle );
	Update_Error_Status( ODBCEnvironmentOperationType::ALLOCATE_CONNECTION_HANDLE, error_code );
	if ( !Was_Last_ODBC_Operation_Successful() )
	{
		return nullptr;
	}

	DBConnectionIDType new_id = NextConnectionID;
	NextConnectionID = static_cast< DBConnectionIDType >( NextConnectionID + 1 );
	IDatabaseConnection *connection = new CODBCConnection( new_id, EnvironmentHandle, dbc_handle, cache_statements );
	connection->Initialize( connection_string );

	Connections[ new_id ] = connection;

	return connection;
}

void CODBCEnvironment::Update_Error_Status( ODBCEnvironmentOperationType operation_type, SQLRETURN error_code )
{
	if ( Refresh_Error_Status( error_code ) )
	{
		return;
	}

	switch ( operation_type )
	{
		case ODBCEnvironmentOperationType::SET_VERSION:
			Set_Error_State_Base( DBEST_FATAL_ERROR );
			break;

		case ODBCEnvironmentOperationType::ALLOCATE_CONNECTION_HANDLE:
			Set_Error_State_Base( DBEST_RECOVERABLE_ERROR );
			break;

		default:
			FATAL_ASSERT( false );
			break;
	}
}

bool CODBCEnvironment::Was_Last_ODBC_Operation_Successful( void ) const
{
	return Was_Database_Operation_Successful( Get_Error_State_Base() );
}

} // namespace Db
} // namespace IP