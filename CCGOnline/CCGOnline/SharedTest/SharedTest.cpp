/**********************************************************************************************************************

	SharedTest.cpp
		the entry point for the console application that runs all the shared library tests

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

#include "stdafx.h"

#include "Shared.h"
#include "GeneratedCode/RegisterSharedTestEnums.h"

namespace NSharedTest
{
	void Initialize( void )
	{
		NShared::Initialize();
		Register_SharedTest_Enums();
	}

	void Shutdown( void )
	{
		NShared::Shutdown();
	}
}

int main(int argc, wchar_t* argv[])
{
	NSharedTest::Initialize();

	::testing::InitGoogleTest(&argc, argv);
	RUN_ALL_TESTS();

	NSharedTest::Shutdown();

	return 0;
}

