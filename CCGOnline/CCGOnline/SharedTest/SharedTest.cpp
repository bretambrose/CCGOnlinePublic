/**********************************************************************************************************************

	[Placeholder for eventual source license]

	SharedTest.cpp
		the entry point for the console application that runs all the shared library tests

	(c) Copyright 2011, Bret Ambrose.  All rights reserved.

**********************************************************************************************************************/

#include "stdafx.h"

#include "Shared.h"

int main(int argc, wchar_t* argv[])
{
	NShared::Initialize();

	::testing::InitGoogleTest(&argc, argv);
	RUN_ALL_TESTS();

	NShared::Shutdown();

	return 0;
}

