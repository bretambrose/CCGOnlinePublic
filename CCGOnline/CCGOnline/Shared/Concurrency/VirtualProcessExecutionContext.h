/**********************************************************************************************************************

	VirtualProcessExecutionContext.h
		A component defining a helper class that contains information about the current execution context of a virtual 
		process.  Processes that are being run under a tbb task can use this to split/subdivide work as necessary.

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

#ifndef VIRTUAL_PROCESS_EXECUTION_CONTEXT_H
#define VIRTUAL_PROCESS_EXECUTION_CONTEXT_H

namespace tbb
{
	class task;
}

// Defines a virtual process's execution context; currently only contains TBB info
class CVirtualProcessExecutionContext
{
	public:

		CVirtualProcessExecutionContext( tbb::task *spawning_task ) :
			SpawningTask( spawning_task )
		{
		}

		~CVirtualProcessExecutionContext() {}

		tbb::task *Get_Spawning_Task( void ) const { return SpawningTask; }

	private:

		tbb::task *SpawningTask;
};

#endif // VIRTUAL_PROCESS_EXECUTION_CONTEXT_H