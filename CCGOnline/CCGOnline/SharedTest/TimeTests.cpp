/**********************************************************************************************************************

	[Placeholder for eventual source license]

	TimeTests.cpp
		defines unit tests for time related functionality

	(c) Copyright 2011, Bret Ambrose.  All rights reserved.

**********************************************************************************************************************/

#include "stdafx.h"

#include "PlatformTime.h"
#include "Time/TickTime.h"
#include "Time/TimeKeeper.h"
#include "Time/TimeUtils.h"
#include "Time/TimeType.h"

class TimeTests : public testing::Test 
{
	protected:  


};

TEST_F( TimeTests, TickTime_Constructors_And_Comparisons )
{
	STickTime time1( 5 );
	STickTime time2( time1 );
	STickTime time3 = time2;

	ASSERT_TRUE( time1 == time2 && time2 == time3 );
	ASSERT_TRUE( time1.Get_Ticks() == 5 );
	ASSERT_FALSE( time1 != time2 );

	STickTime time4( 3 );

	ASSERT_TRUE( time4 < time1 );
}

TEST_F( TimeTests, TickTime_Arithmetic )
{
	STickTime time1( 1 );
	STickTime time2( 2 );

	STickTime time3 = time1 + time2;
	STickTime time4( time1 );
	time4 += time2;

	ASSERT_TRUE( time3 == time4 );
	ASSERT_TRUE( time3.Get_Ticks() == 3 );

	time4 -= time2;
	ASSERT_TRUE( time4 == time1 );

	STickTime time5 = time3 - time2;
	ASSERT_TRUE( time5 == time1 );
}

TEST_F( TimeTests, TimeUtils_Convert_GameTicks_To_Seconds )
{
	STickTime game_time( 5000 );
	ASSERT_TRUE( NTimeUtils::Convert_Game_Ticks_To_Seconds( game_time ) == 5.0 );
	ASSERT_TRUE( NTimeUtils::Convert_Ticks_To_Seconds( TT_GAME_TIME, game_time ) == 5.0 );
}

TEST_F( TimeTests, TimeUtils_Convert_RealTicks_To_Seconds )
{
	STickTime real_time( 5000000 );
	double real_time_seconds = CPlatformTime::Convert_High_Resolution_Time_To_Seconds( real_time.Get_Ticks() );

	ASSERT_TRUE( NTimeUtils::Convert_High_Resolution_Ticks_To_Seconds( real_time ) == real_time_seconds );
	ASSERT_TRUE( NTimeUtils::Convert_Ticks_To_Seconds( TT_REAL_TIME, real_time ) == real_time_seconds );
}

TEST_F( TimeTests, TimeUtils_Convert_Seconds_To_GameTicks )
{
	STickTime game_time( 5000 );
	ASSERT_TRUE( NTimeUtils::Convert_Seconds_To_Game_Ticks( 5.0 ) == game_time );
	ASSERT_TRUE( NTimeUtils::Convert_Seconds_To_Ticks( TT_GAME_TIME, 5.0 ) == game_time );

}

TEST_F( TimeTests, TimeUtils_Convert_Seconds_To_RealTicks )
{
	STickTime real_time( 5000000 );
	double real_time_seconds = CPlatformTime::Convert_High_Resolution_Time_To_Seconds( real_time.Get_Ticks() );

	ASSERT_TRUE( NTimeUtils::Convert_Seconds_To_High_Resolution_Ticks( real_time_seconds ) == real_time );
	ASSERT_TRUE( NTimeUtils::Convert_Seconds_To_Ticks( TT_REAL_TIME, real_time_seconds ) == real_time );
}

TEST_F( TimeTests, TimeKeeper_GameTime )
{
	CTimeKeeper time_keeper;

	time_keeper.Set_Base_Time( TT_GAME_TIME, STickTime( 5000 ) );

	time_keeper.Set_Current_Time( TT_GAME_TIME, STickTime( 7000 ) );
	double elapsed_seconds = time_keeper.Get_Elapsed_Seconds( TT_GAME_TIME );
	ASSERT_TRUE( elapsed_seconds == 2.0 && NTimeUtils::Convert_Game_Ticks_To_Seconds( time_keeper.Get_Elapsed_Ticks( TT_GAME_TIME ) ) == 2.0 );
}

TEST_F( TimeTests, TimeKeeper_RealTime )
{
	static const uint64 TEST_HR_TICKS1 = 10000;
	static const uint64 TEST_HR_TICKS2 = 1000000;

	CTimeKeeper time_keeper;

	time_keeper.Set_Base_Time( TT_REAL_TIME, STickTime( TEST_HR_TICKS1 ) );

	double real_seconds = NTimeUtils::Convert_High_Resolution_Ticks_To_Seconds( STickTime( TEST_HR_TICKS2 - TEST_HR_TICKS1 ) );
	time_keeper.Set_Current_Time( TT_REAL_TIME, STickTime( TEST_HR_TICKS2 ) );
	double elapsed_seconds = time_keeper.Get_Elapsed_Seconds( TT_REAL_TIME );
	ASSERT_TRUE( elapsed_seconds == real_seconds && NTimeUtils::Convert_High_Resolution_Ticks_To_Seconds( time_keeper.Get_Elapsed_Ticks( TT_REAL_TIME ) ) == real_seconds );
}