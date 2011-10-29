/**********************************************************************************************************************

	[Placeholder for eventual source license]

	TimeUtils.h
		A component defining several utility functions for converting between different fundamental time types
			used in the library

	(c) Copyright 2011, Bret Ambrose.  All rights reserved.

**********************************************************************************************************************/

#ifndef TIME_UTILS_H
#define TIME_UTILS_H

struct STickTime;

enum ETimeType;

namespace NTimeUtils
{
	double Convert_Ticks_To_Seconds( ETimeType time_type, const STickTime &tick_time );
	STickTime Convert_Seconds_To_Ticks( ETimeType time_type, double seconds );

	double Convert_High_Resolution_Ticks_To_Seconds( const STickTime &high_res_time );
	STickTime Convert_Seconds_To_High_Resolution_Ticks( double seconds );

	double Convert_Game_Ticks_To_Seconds( const STickTime &game_time );
	STickTime Convert_Seconds_To_Game_Ticks( double seconds );
}

#endif // TIME_UTILS_H
