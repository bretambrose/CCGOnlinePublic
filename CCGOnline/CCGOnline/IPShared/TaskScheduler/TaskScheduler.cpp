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

#include "TaskScheduler.h"

#include "ScheduledTask.h"
#include "ScheduledTaskPolicies.h"
#include "IPShared/PriorityQueue.h"
#include <limits.h>

static const double TIME_GRANULARITY_FRACTION_CUTOFF = .00001;


CTaskScheduler::CTaskScheduler( void ) :
	TaskQueue( new TPriorityQueue< shared_ptr< CScheduledTask >, CScheduledTaskMovementPolicy, CScheduledTaskComparator >() ),
	TimeGranularity( 0.0 )
{
}


CTaskScheduler::CTaskScheduler( double time_granularity ) :
	TaskQueue( new TPriorityQueue< shared_ptr< CScheduledTask >, CScheduledTaskMovementPolicy, CScheduledTaskComparator >() ),
	TimeGranularity( time_granularity )
{
}


CTaskScheduler::~CTaskScheduler()
{
}


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


void CTaskScheduler::Submit_Task( const shared_ptr< CScheduledTask > &task )
{
	FATAL_ASSERT( !task->Is_Scheduled() );

	if ( TimeGranularity > 0.0 )
	{
		// Round the desired execution time to the nearest granule
		double fractional_granules = task->Get_Execute_Time() / TimeGranularity;
		double granules = static_cast< double >( static_cast< uint64_t >( fractional_granules ) );
		if ( fractional_granules - granules > TIME_GRANULARITY_FRACTION_CUTOFF )
		{
			double new_time = ( granules + 1 ) * TimeGranularity;
			task->Set_Execute_Time( new_time );
		}
	}

	TaskQueue->Insert( task );
}


void CTaskScheduler::Remove_Task( const shared_ptr< CScheduledTask > &task )
{
	TaskQueue->Remove_By_Index( task->Get_Heap_Index() );
}


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

