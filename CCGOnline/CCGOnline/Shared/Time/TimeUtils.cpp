/**********************************************************************************************************************

	[Placeholder for eventual source license]

	TimeUtils.cpp
		A component defining several utility functions for converting between different fundamental time types
			used in the library

	(c) Copyright 2011, Bret Ambrose.  All rights reserved.

**********************************************************************************************************************/

#include "stdafx.h"

#include "TimeUtils.h"

#include "TickTime.h"
#include "TimeType.h"
#include "PlatformTime.h"

static const double GAME_TICKS_PER_SECOND = 1000.0;

/**********************************************************************************************************************
	NTimeUtils::Convert_Ticks_To_Seconds -- function to convert from a tick time to a time in seconds
	
		time_type -- the type of time to convert
		tick_time -- the number of ticks in the time value
		
		Returns: the corresponding time in seconds
		
**********************************************************************************************************************/
double NTimeUtils::Convert_Ticks_To_Seconds( ETimeType time_type, const STickTime &tick_time )
{
	if ( time_type == TT_REAL_TIME )
	{
		return Convert_High_Resolution_Ticks_To_Seconds( tick_time );
	}
	else
	{
		return Convert_Game_Ticks_To_Seconds( tick_time );
	}
}

/**********************************************************************************************************************
	NTimeUtils::Convert_Seconds_To_Ticks -- function to convert from a time in seconds to a time in ticks
	
		time_type -- the type of time to convert to
		seconds -- the number of seconds in the time value
		
		Returns: the corresponding time in ticks
		
**********************************************************************************************************************/
STickTime NTimeUtils::Convert_Seconds_To_Ticks( ETimeType time_type, double seconds )
{
	if ( time_type == TT_REAL_TIME )
	{
		return Convert_Seconds_To_High_Resolution_Ticks( seconds );
	}
	else
	{
		return Convert_Seconds_To_Game_Ticks( seconds );
	}
}

/**********************************************************************************************************************
	NTimeUtils::Convert_High_Resolution_Ticks_To_Seconds -- function to convert from a HR tick time to a time in seconds
	
		tick_time -- the number of high resolution ticks in the time value
		
		Returns: the corresponding time in seconds
		
**********************************************************************************************************************/
double NTimeUtils::Convert_High_Resolution_Ticks_To_Seconds( const STickTime &high_res_time )
{
	return CPlatformTime::Convert_High_Resolution_Time_To_Seconds( high_res_time.Get_Ticks() );
}

/**********************************************************************************************************************
	NTimeUtils::Convert_Seconds_To_High_Resolution_Ticks -- function to convert from a time in seconds to a time in HR ticks
	
		seconds -- the time in seconds to convert
		
		Returns: the corresponding time in high resolution ticks
		
**********************************************************************************************************************/
STickTime NTimeUtils::Convert_Seconds_To_High_Resolution_Ticks( double seconds )
{
	return STickTime( CPlatformTime::Convert_Seconds_To_High_Resolution_Ticks( seconds ) );
}

/**********************************************************************************************************************
	NTimeUtils::Convert_Game_Ticks_To_Seconds -- function to convert from a game tick time to a time in seconds
	
		tick_time -- the number of game ticks in the time value
		
		Returns: the corresponding time in seconds
		
**********************************************************************************************************************/
double NTimeUtils::Convert_Game_Ticks_To_Seconds( const STickTime &game_time )
{
	return static_cast< double >( game_time.Get_Ticks() ) / GAME_TICKS_PER_SECOND;
}

/**********************************************************************************************************************
	NTimeUtils::Convert_Seconds_To_Game_Ticks -- function to convert from a time in seconds to a time in game ticks
	
		seconds -- the time in seconds to convert
		
		Returns: the corresponding time in game ticks
		
**********************************************************************************************************************/
STickTime NTimeUtils::Convert_Seconds_To_Game_Ticks( double seconds )
{
	return STickTime( static_cast< uint64 >( seconds * GAME_TICKS_PER_SECOND ) );
}



