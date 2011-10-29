/**********************************************************************************************************************

	[Placeholder for eventual source license]

	TaskScheduler.cpp
		A component defining a class that tracks a set of tasks to be executed in the future.  As time advances, the
		tasks are executed appropriately.

	(c) Copyright 2011, Bret Ambrose.  All rights reserved.

**********************************************************************************************************************/

#include "stdafx.h"

#include "TaskScheduler.h"

#include "ScheduledTask.h"
#include "ScheduledTaskPolicies.h"
#include "PriorityQueue.h"
#include <limits.h>

static const double TIME_GRANULARITY_FRACTION_CUTOFF = .00001;

/**********************************************************************************************************************
	CTaskScheduler::CTaskScheduler -- default constructor
		
**********************************************************************************************************************/
CTaskScheduler::CTaskScheduler( void ) :
	TaskQueue( new TPriorityQueue< shared_ptr< CScheduledTask >, CScheduledTaskMovementPolicy, CScheduledTaskComparator >() ),
	TimeGranularity( 0.0 )
{
}

/**********************************************************************************************************************
	CTaskScheduler::CTaskScheduler -- constructor
		
		time_granularity -- all scheduled tasks will have their execution time rounded to this granularity
		
**********************************************************************************************************************/
CTaskScheduler::CTaskScheduler( double time_granularity ) :
	TaskQueue( new TPriorityQueue< shared_ptr< CScheduledTask >, CScheduledTaskMovementPolicy, CScheduledTaskComparator >() ),
	TimeGranularity( time_granularity )
{
}

/**********************************************************************************************************************
	CTaskScheduler::~CTaskScheduler -- destructor
	
	Note: Intentionally placed here rather than the header file so that cpp files using TaskScheduler objects do not
	need to include priority queue as well	
**********************************************************************************************************************/
CTaskScheduler::~CTaskScheduler()
{
}

/**********************************************************************************************************************
	CTaskScheduler:Get_Next_Task_Time -- queries the execution time for the next task to be executed; used to 
		potentially aid thread sleep time calculations

		Returns: the execution time, in seconds, of the next task to be executed
		
**********************************************************************************************************************/
double CTaskScheduler::Get_Next_Task_Time( void ) const
{
	shared_ptr< CScheduledTask > task;
	TaskQueue->Peek_Top( task );
	if ( task != nullptr )
	{
		return task->Get_Execute_Time();
	}
	else
	{
		return std::numeric_limits< double >::max();
	}
}

/**********************************************************************************************************************
	CTaskScheduler:Submit_Task -- submits a task to the scheduler for future execution

		task -- task to be executed
		
**********************************************************************************************************************/
void CTaskScheduler::Submit_Task( const shared_ptr< CScheduledTask > &task )
{
	FATAL_ASSERT( !task->Is_Scheduled() );

	if ( TimeGranularity > 0.0 )
	{
		// Round the desired execution time to the nearest granule
		double fractional_granules = task->Get_Execute_Time() / TimeGranularity;
		double granules = static_cast< double >( static_cast< uint64 >( fractional_granules ) );
		if ( fractional_granules - granules > TIME_GRANULARITY_FRACTION_CUTOFF )
		{
			double new_time = ( granules + 1 ) * TimeGranularity;
			task->Set_Execute_Time( new_time );
		}
	}

	TaskQueue->Insert( task );
}

/**********************************************************************************************************************
	CTaskScheduler:Remove_Task -- removes a task from the scheduler prior to execution

		task -- task to be removed
		
**********************************************************************************************************************/
void CTaskScheduler::Remove_Task( const shared_ptr< CScheduledTask > &task )
{
	TaskQueue->Remove_By_Index( task->Get_Heap_Index() );
}

/**********************************************************************************************************************
	CTaskScheduler:Service -- executes all tasks whose execution time is at or before the supplied current time

		current_time_seconds -- the cutoff time, in seconds, for executing tasks
		
**********************************************************************************************************************/
void CTaskScheduler::Service( double current_time_seconds )
{
	shared_ptr< CScheduledTask > task;
	while ( TaskQueue->Peek_Top( task ) && task != nullptr && task->Get_Execute_Time() <= current_time_seconds )
	{
		TaskQueue->Pop();

		double reschedule_time;
		// returning true indicates that the task should be rescheduled
		if ( task->Execute( current_time_seconds, reschedule_time ) )
		{
			// prevent potential reschedule loops
			FATAL_ASSERT( reschedule_time > current_time_seconds );

			task->Set_Execute_Time( reschedule_time );
			Submit_Task( task );
		}
	}
}

