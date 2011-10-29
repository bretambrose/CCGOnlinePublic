/**********************************************************************************************************************

	[Placeholder for eventual source license]

	ScheduledTask.h
		A component defining the base class for all scheduled tasks.

	(c) Copyright 2011, Bret Ambrose.  All rights reserved.

**********************************************************************************************************************/

#ifndef SCHEDULED_TASK_H
#define SCHEDULED_TASK_H

// A base class to be used by all schedule task objects
class CScheduledTask
{
	public:

		CScheduledTask( double execute_time_seconds ) :
			ExecuteTimeSeconds( execute_time_seconds ),
			HeapIndex( 0 )
		{}

		virtual ~CScheduledTask() {}

		virtual bool Execute( double current_time_seconds, double &reschedule_time_seconds ) = 0;

		double Get_Execute_Time( void ) const { return ExecuteTimeSeconds; }
		void Set_Execute_Time( double execute_time_seconds ) { ExecuteTimeSeconds = execute_time_seconds; }

		size_t Get_Heap_Index( void ) const { return HeapIndex; }
		void Set_Heap_Index( size_t heap_index ) { HeapIndex = heap_index; }

		bool Is_Scheduled( void ) const { return HeapIndex > 0; }

	private:

		double ExecuteTimeSeconds;

		size_t HeapIndex;

};


#endif // SCHEDULED_TASK_H
