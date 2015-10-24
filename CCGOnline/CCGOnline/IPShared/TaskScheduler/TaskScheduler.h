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

#pragma once

namespace IP
{
namespace Algorithm
{

template< typename T1, typename T2, typename T3 > class TPriorityQueue;

} // namespace Algorithm

namespace Execution
{

class CScheduledTask;
class CScheduledTaskMovementPolicy;
class CScheduledTaskComparator;


// A class that tracks and executes time-scheduled tasks
class CTaskScheduler
{
	public:

		CTaskScheduler( void );
		CTaskScheduler( double time_granularity );
		~CTaskScheduler();

		void Submit_Task( const std::shared_ptr< CScheduledTask > &task );
		void Remove_Task( const std::shared_ptr< CScheduledTask > &task );

		void Service( double current_time_seconds );

		double Get_Time_Granularity( void ) const { return TimeGranularity; }

		double Get_Next_Task_Time( void ) const;

	private:

		std::unique_ptr< IP::Algorithm::TPriorityQueue< std::shared_ptr< CScheduledTask >, CScheduledTaskMovementPolicy, CScheduledTaskComparator > > TaskQueue;

		double TimeGranularity;
};

} // namespace Execution
} // namespace IP
