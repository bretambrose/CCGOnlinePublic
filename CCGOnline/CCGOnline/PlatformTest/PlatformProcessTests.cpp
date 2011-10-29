/**********************************************************************************************************************

	[Placeholder for eventual source license]

	PlatformProcessTests.cpp
		defines unit tests for process-related platform-specific functionality

	(c) Copyright 2011, Bret Ambrose.  All rights reserved.

**********************************************************************************************************************/

#include "stdafx.h"

#include <fstream>
#include <iostream>

#include "PlatformProcess.h"

class PlatformProcessTests : public testing::Test 
{
	protected:  

		static void SetUpTestCase( void ) 
		{			
		}

		static void TearDownTestCase( void )
		{
		}

	private:


};

TEST_F( PlatformProcessTests, Exe_Name )
{
	ASSERT_TRUE( NPlatform::Get_Service_Name() == L"PlatformTest" );
}

