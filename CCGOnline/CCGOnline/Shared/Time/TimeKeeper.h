/**********************************************************************************************************************

	[Placeholder for eventual source license]

	TimeKeeper.h
		A component defining a class that tracks tick times for multiple time scales

	(c) Copyright 2011, Bret Ambrose.  All rights reserved.

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

		typedef stdext::hash_map< ETimeType, STickTime > TimeTableType;

		TimeTableType CurrentTimes;
		TimeTableType BaseTimes;
};

#endif // TIME_KEEPER_H
