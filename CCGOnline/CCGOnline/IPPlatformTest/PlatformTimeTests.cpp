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

#include "stdafx.h"

#include <fstream>
#include <iostream>

#include "IPPlatform/PlatformTime.h"
#include "IPPlatform/PlatformProcess.h"

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
	uint64_t time1 = CPlatformTime::Get_Raw_Time();

	std::this_thread::sleep_for( std::chrono::milliseconds( 1500 ) );

	uint64_t time2 = CPlatformTime::Get_Raw_Time();

	ASSERT_TRUE( CPlatformTime::Is_Raw_Time_Less_Than_Seconds( time1, time2, 0 ) );
	ASSERT_TRUE( CPlatformTime::Is_Raw_Time_Less_Than_Seconds( time1, time2, 1 ) );
	ASSERT_FALSE( CPlatformTime::Is_Raw_Time_Less_Than_Seconds( time1, time2, 10 ) );
}
