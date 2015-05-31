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

#include "TimeUtils.h"

#include "TickTime.h"
#include "TimeType.h"
#include "IPPlatform/PlatformTime.h"

static const double GAME_TICKS_PER_SECOND = 1000.0;


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


double NTimeUtils::Convert_High_Resolution_Ticks_To_Seconds( const STickTime &high_res_time )
{
	return CPlatformTime::Convert_High_Resolution_Time_To_Seconds( high_res_time.Get_Ticks() );
}


STickTime NTimeUtils::Convert_Seconds_To_High_Resolution_Ticks( double seconds )
{
	return STickTime( CPlatformTime::Convert_Seconds_To_High_Resolution_Ticks( seconds ) );
}


double NTimeUtils::Convert_Game_Ticks_To_Seconds( const STickTime &game_time )
{
	return static_cast< double >( game_time.Get_Ticks() ) / GAME_TICKS_PER_SECOND;
}


STickTime NTimeUtils::Convert_Seconds_To_Game_Ticks( double seconds )
{
	return STickTime( static_cast< uint64_t >( seconds * GAME_TICKS_PER_SECOND ) );
}



