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
