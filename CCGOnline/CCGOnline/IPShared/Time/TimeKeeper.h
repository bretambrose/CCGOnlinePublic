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

#ifndef TIME_KEEPER_H
#define TIME_KEEPER_H

struct STickTime;
enum ETimeType;

// Class that tracks the current tick times for one or more time types
class CTimeKeeper
{
	public:

		CTimeKeeper( void );
		~CTimeKeeper() {}

		void Set_Current_Time( ETimeType time_type, const STickTime &current_time );
		void Set_Base_Time( ETimeType time_type, const STickTime &current_time );

		STickTime Get_Elapsed_Ticks( ETimeType time_type ) const;
		double Get_Elapsed_Seconds( ETimeType time_type ) const;

	private:

		const STickTime &Get_Base_Time( ETimeType time_type ) const;
		const STickTime &Get_Current_Time( ETimeType time_type ) const;

		typedef std::unordered_map< ETimeType, STickTime > TimeTableType;

		TimeTableType CurrentTimes;
		TimeTableType BaseTimes;
};

#endif // TIME_KEEPER_H
