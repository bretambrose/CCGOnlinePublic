/**********************************************************************************************************************

	[Placeholder for eventual source license]

	TaskScheduler.h
		A component defining a class that tracks a set of tasks to be executed in the future.  As time advances, the
		tasks are executed appropriately.

	(c) Copyright 2011, Bret Ambrose.  All rights reserved.

**********************************************************************************************************************/

#ifndef TASK_SCHEDULER_H
#define TASK_SCHEDULER_H

class CScheduledTask;
class CScheduledTaskMovementPolicy;
class CScheduledTaskComparator;
struct STickTime;

template< typename T1, typename T2, typename T3 > class TPriorityQueue;

// A class that tracks and executes time-scheduled tasks
class CTaskScheduler
{
	public:

		CTaskScheduler( void );
		CTaskScheduler( double time_granularity );
		~CTaskScheduler();

		void Submit_Task( const shared_ptr< CScheduledTask > &task );
		void Remove_Task( const shared_ptr< CScheduledTask > &task );

		void Service( double current_time_seconds );

		double Get_Time_Granularity( void ) const { return TimeGranularity; }

		double Get_Next_Task_Time( void ) const;

	private:

		scoped_ptr< TPriorityQueue< shared_ptr< CScheduledTask >, CScheduledTaskMovementPolicy, CScheduledTaskComparator > > TaskQueue;

		double TimeGranularity;
};

#endif // TASK_SCHEDULER_H
