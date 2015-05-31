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

#ifndef SCHEDULED_TASK_POLICIES_H
#define SCHEDULED_TASK_POLICIES_H

#include "ScheduledTask.h"

// a priority queue policy for scheduled tasks that adds internal heap index tracking, allowing
// for efficient ( O( Log N ) ) removes of cancelled tasks.
class CScheduledTaskMovementPolicy
{
	public:

		static void Swap( std::shared_ptr< CScheduledTask > &lhs, std::shared_ptr< CScheduledTask > &rhs )
		{
			if ( lhs.get() != rhs.get() )
			{
				std::shared_ptr< CScheduledTask > temp( lhs );
				lhs = rhs;
				rhs = temp;

				size_t temp_index = lhs->Get_Heap_Index();
				lhs->Set_Heap_Index( rhs->Get_Heap_Index() );
				rhs->Set_Heap_Index( temp_index );
			}
		}

		static void Set_Index( const std::shared_ptr< CScheduledTask > &task, size_t index )
		{
			task->Set_Heap_Index( index );
		}
};

// function object that allows comparison of two tasks by their desired execution times
class CScheduledTaskComparator
{
	public:

		bool operator ()( const std::shared_ptr< CScheduledTask > &task1, const std::shared_ptr< CScheduledTask > &task2 ) const
		{
			return task1->Get_Execute_Time() < task2->Get_Execute_Time();
		}
};

#endif // SCHEDULED_TASK_POLICIES_H