/**********************************************************************************************************************

	[Placeholder for eventual source license]

	PlatformTimeTests.cpp
		Unit tests for platform time functionality

	(c) Copyright 2011, Bret Ambrose.  All rights reserved.

**********************************************************************************************************************/

#include "stdafx.h"

#include <fstream>
#include <iostream>

#include "PlatformTime.h"
#include "PlatformProcess.h"

class PlatformTimeTests : public testing::Test 
{
	protected:  

		static void SetUpTestCase( void ) 
		{
			CPlatformTime::Initialize();			
		}

		static void TearDownTestCase( void )
		{
		}

	private:


};

TEST_F( PlatformTimeTests, Raw_Time_Comparisons )
{
	uint64 time1 = CPlatformTime::Get_Raw_Time();

	NPlatform::Sleep( 1500 );

	uint64 time2 = CPlatformTime::Get_Raw_Time();

	ASSERT_TRUE( CPlatformTime::Is_Raw_Time_Less_Than_Seconds( time1, time2, 0 ) );
	ASSERT_TRUE( CPlatformTime::Is_Raw_Time_Less_Than_Seconds( time1, time2, 1 ) );
	ASSERT_FALSE( CPlatformTime::Is_Raw_Time_Less_Than_Seconds( time1, time2, 10 ) );
}