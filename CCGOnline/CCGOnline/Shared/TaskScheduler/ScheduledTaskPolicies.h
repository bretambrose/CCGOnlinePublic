/**********************************************************************************************************************

	[Placeholder for eventual source license]

	ScheduledTaskPolicies.h
		A component defining a utility classes for task schedulers

	(c) Copyright 2011, Bret Ambrose.  All rights reserved.

**********************************************************************************************************************/

#ifndef SCHEDULED_TASK_POLICIES_H
#define SCHEDULED_TASK_POLICIES_H

#include "ScheduledTask.h"

// a priority queue policy for scheduled tasks that adds internal heap index tracking, allowing
// for efficient ( O( Log N ) ) removes of cancelled tasks.
class CScheduledTaskMovementPolicy
{
	public:

		static void Swap( shared_ptr< CScheduledTask > &lhs, shared_ptr< CScheduledTask > &rhs )
		{
			if ( lhs.get() != rhs.get() )
			{
				shared_ptr< CScheduledTask > temp( lhs );
				lhs = rhs;
				rhs = temp;

				size_t temp_index = lhs->Get_Heap_Index();
				lhs->Set_Heap_Index( rhs->Get_Heap_Index() );
				rhs->Set_Heap_Index( temp_index );
			}
		}

		static void Set_Index( const shared_ptr< CScheduledTask > &task, size_t index )
		{
			task->Set_Heap_Index( index );
		}
};

// function object that allows comparison of two tasks by their desired execution times
class CScheduledTaskComparator
{
	public:

		bool operator ()( const shared_ptr< CScheduledTask > &task1, const shared_ptr< CScheduledTask > &task2 ) const
		{
			return task1->Get_Execute_Time() < task2->Get_Execute_Time();
		}
};

#endif // SCHEDULED_TASK_POLICIES_H