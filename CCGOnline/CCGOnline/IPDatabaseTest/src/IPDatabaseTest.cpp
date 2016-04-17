/**********************************************************************************************************************

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

#include <IPDatabase/IPDatabaseGlobal.h>
#include <IPShared/IPSharedGlobal.h>
#include <gtest/gtest.h>

#include <IPDatabase/ODBCImplementation/ODBCFactory.h>

#include <IPCore/Memory/Memory.h>

#include <IPCore/Debug/DebugAssert.h>
#include <Windows.h>
#include <sql.h>
#include <sqlext.h>

namespace IP
{
namespace Global
{

void Initialize_IPDatabaseTest( void )
{
	Initialize_IPShared();
	Initialize_IPDatabase();

	SQLHENV environment_handle = 0;
	SQLRETURN error_code = SQLAllocHandle( SQL_HANDLE_ENV, SQL_NULL_HANDLE, &environment_handle );
	FATAL_ASSERT( error_code == SQL_SUCCESS );

	error_code = SQLSetEnvAttr( environment_handle, SQL_ATTR_ODBC_VERSION, (void*) SQL_OV_ODBC3, 0 );
	FATAL_ASSERT( error_code == SQL_SUCCESS );

	error_code = SQLFreeHandle( SQL_HANDLE_ENV, environment_handle );
	FATAL_ASSERT( error_code == SQL_SUCCESS );

	auto things = IP::New_Array< uint64_t >( MEMORY_TAG, 200 );
	IP::Delete_Array( things );

	IP::Db::CODBCFactory::Create_Environment();

	things = IP::New_Array< uint64_t >( MEMORY_TAG, 200 );
	IP::Delete_Array( things );

	IP::Db::CODBCFactory::Destroy_Environment();
}

void Shutdown_IPDatabaseTest( void )
{
	Shutdown_IPDatabase();
	Shutdown_IPShared();
}

} // namespace Global
} // namespace IP

using namespace IP::Global;

int main(int argc, char* argv[])
{
	Initialize_IPDatabaseTest();


	::testing::InitGoogleTest(&argc, argv);
	int result_code = RUN_ALL_TESTS();

	Shutdown_IPDatabaseTest();

	return result_code;
}

