/**********************************************************************************************************************

	[Placeholder for eventual source license]

	PlatformTest.cpp
		the entry point for the console application that runs the unit tests

	(c) Copyright 2011, Bret Ambrose.  All rights reserved.

**********************************************************************************************************************/

#include "stdafx.h"

int main(int argc, wchar_t* argv[])
{
	::testing::InitGoogleTest(&argc, argv);
	RUN_ALL_TESTS();

	return 0;
}


