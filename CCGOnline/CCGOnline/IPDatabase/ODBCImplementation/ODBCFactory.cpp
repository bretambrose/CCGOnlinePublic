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

#include "ODBCFactory.h"
#include "ODBCEnvironment.h"

namespace IP
{
namespace Db
{

IDatabaseEnvironment *CODBCFactory::Environment( nullptr );

void CODBCFactory::Create_Environment( void )
{
	if ( Environment != nullptr )
	{
		return;
	}

	SQLHENV environment_handle = 0;
	SQLRETURN error_code = SQLAllocHandle( SQL_HANDLE_ENV, SQL_NULL_HANDLE, &environment_handle );
	FATAL_ASSERT( error_code == SQL_SUCCESS );

	Environment = new CODBCEnvironment( environment_handle );
	Environment->Initialize();
}

void CODBCFactory::Destroy_Environment( void )
{
	if ( Environment != nullptr )
	{
		Environment->Shutdown();
		delete Environment;
		Environment = nullptr;
	}
}

IDatabaseEnvironment *CODBCFactory::Get_Environment( void )
{
	return Environment;
}

} // namespace Db
} // namespace IP

