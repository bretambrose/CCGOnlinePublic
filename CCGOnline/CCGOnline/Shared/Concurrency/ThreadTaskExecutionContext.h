/**********************************************************************************************************************

	[Placeholder for eventual source license]

	ThreadTaskExecutionContext.h
		A component defining a helper class that contains information about the current execution context of a thread
		task.  Tasks that are being run under a tbb task can use this to split/subdivide work as necessary.

	(c) Copyright 2011, Bret Ambrose.  All rights reserved.

**********************************************************************************************************************/

#ifndef THREAD_TASK_EXECUTION_CONTEXT_H
#define THREAD_TASK_EXECUTION_CONTEXT_H

namespace tbb
{
	class task;
}

// Defines a Thread task's execution context; currently only contains TBB info
class CThreadTaskExecutionContext
{
	public:

		CThreadTaskExecutionContext( tbb::task *spawning_task ) :
			SpawningTask( spawning_task )
		{
		}

		~CThreadTaskExecutionContext() {}

		tbb::task *Get_Spawning_Task( void ) const { return SpawningTask; }

	private:

		tbb::task *SpawningTask;
};

#endif // THREAD_TASK_EXECUTION_CONTEXT_H