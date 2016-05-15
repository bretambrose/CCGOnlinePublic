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

#include <IPShared/IPSharedGlobal.h>
#include <IPSharedTest/GeneratedCode/RegisterIPSharedTestEnums.h>
#include <gtest/gtest.h>

namespace NIPSharedTest
{
	void Initialize( void )
	{
		IP::Global::Initialize_IPShared();
		IP::Global::Register_IPSharedTest_Enums();
	}

	void Shutdown( void )
	{
		IP::Global::Shutdown_IPShared();
	}
}

int main(int argc, char* argv[])
{
	NIPSharedTest::Initialize();

	::testing::InitGoogleTest(&argc, argv);
	int result_code = RUN_ALL_TESTS();

	NIPSharedTest::Shutdown();

	return result_code;
}
