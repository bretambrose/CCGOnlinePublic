/**********************************************************************************************************************

	[Placeholder for eventual source license]

	TimeKeeper.cpp
		A component defining a class that tracks tick times for multiple time scales

	(c) Copyright 2011, Bret Ambrose.  All rights reserved.

**********************************************************************************************************************/

#include "stdafx.h"

#include "TimeKeeper.h"

#include "Time/TimeType.h"
#include "Time/TickTime.h"
#include "Time/TimeUtils.h"

/**********************************************************************************************************************
	CTimeKeeper::CTimeKeeper -- default constructor
		
**********************************************************************************************************************/
CTimeKeeper::CTimeKeeper( void ) :
	CurrentTimes(),
	BaseTimes()
{
	// Reset all time types into a default state
	for ( uint32 tt = static_cast< uint32 >( TT_FIRST ); tt < static_cast< uint32 >( TT_COUNT ); ++tt )
	{
		ETimeType time_type = static_cast< ETimeType >( tt );

		CurrentTimes[ time_type ] = STickTime( static_cast< uint64 >( 0 ) );
		BaseTimes[ time_type ] = STickTime( static_cast< uint64 >( 0 ) );
	}
}

/**********************************************************************************************************************
	CTimeKeeper::Get_Current_Time -- gets the current tick time for a time type

		time_type -- time type to get the current time for

		Returns: current time in ticks for the supplied time type
		
**********************************************************************************************************************/
const STickTime &CTimeKeeper::Get_Current_Time( ETimeType time_type ) const
{
	auto iter = CurrentTimes.find( time_type );
	return iter->second;
}

/**********************************************************************************************************************
	CTimeKeeper::Set_Current_Time -- sets the current tick time for a time type

		time_type -- time type to set the current time for
		current_time -- new current time for that time type
		
**********************************************************************************************************************/
void CTimeKeeper::Set_Current_Time( ETimeType time_type, const STickTime &current_time )
{
	auto iter = CurrentTimes.find( time_type );

	DEBUG_ASSERT( current_time.Get_Ticks() > iter->second.Get_Ticks() || iter->second.Get_Ticks() == 0 );
	iter->second = current_time;
}

/**********************************************************************************************************************
	CTimeKeeper::Get_Base_Time -- gets the base/starting tick time for a time type; used to calculate elapsed time

		time_type -- time type to get the base/starting time for

		Returns: base time in ticks for the supplied time type
		
**********************************************************************************************************************/
const STickTime &CTimeKeeper::Get_Base_Time( ETimeType time_type ) const
{
	auto iter = BaseTimes.find( time_type );
	return iter->second;
}

/**********************************************************************************************************************
	CTimeKeeper::Set_Base_Time -- sets the base/starting tick time for a time type; used to calculate elapsed time

		time_type -- time type to set the base/starting time for
		base_time -- new base time in ticks for the supplied time type
		
**********************************************************************************************************************/
void CTimeKeeper::Set_Base_Time( ETimeType time_type, const STickTime &base_time )
{
	auto iter = BaseTimes.find( time_type );

	DEBUG_ASSERT( base_time.Get_Ticks() > iter->second.Get_Ticks() || iter->second.Get_Ticks() == 0 );
	iter->second = base_time;

	Set_Current_Time( time_type, base_time );
}

/**********************************************************************************************************************
	CTimeKeeper::Get_Elapsed_Ticks -- gets the elapsed time in ticks for a time type

		time_type -- time type to get the elapsed time for

		Returns: elapsed time in ticks for the supplied time type
		
**********************************************************************************************************************/
STickTime CTimeKeeper::Get_Elapsed_Ticks( ETimeType time_type ) const
{
	return Get_Current_Time( time_type ) - Get_Base_Time( time_type );
}

/**********************************************************************************************************************
	CTimeKeeper::Get_Elapsed_Seconds -- gets the elapsed time in seconds for a time type

		time_type -- time type to get the elapsed time for

		Returns: elapsed time in seconds for the supplied time type
		
**********************************************************************************************************************/
double CTimeKeeper::Get_Elapsed_Seconds( ETimeType time_type ) const
{
	return NTimeUtils::Convert_Ticks_To_Seconds( time_type, Get_Current_Time( time_type ) - Get_Base_Time( time_type ) );
}
