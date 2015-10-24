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
namespace Execution
{

// A base class to be used by all schedule task objects
class CScheduledTask
{
	public:

		CScheduledTask( double execute_time_seconds ) :
			ExecuteTimeSeconds( execute_time_seconds ),
			HeapIndex( 0 )
		{}

		virtual ~CScheduledTask() = default;

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

} // namespace Execution
} // namespace IP

