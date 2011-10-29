/**********************************************************************************************************************

	[Placeholder for eventual source license]

	ThreadStatics.h
		A component containing a static class that manages the thread-local variables that hold handles to
		the executing task and the concurrency manager.

	(c) Copyright 2011, Bret Ambrose.  All rights reserved.

**********************************************************************************************************************/

#ifndef THREAD_STATICS_H
#define THREAD_STATICS_H

class IThreadTask;
class CConcurrencyManager;

// Static class managing thread-local handles to the executing task and the concurrency manager. 
class CThreadStatics
{
	public:

		static void Initialize( void );
		static void Shutdown( void );

		static void Set_Current_Thread_Task( IThreadTask *thread_task );
		static void Set_Concurrency_Manager( CConcurrencyManager *manager );

		static IThreadTask *Get_Current_Thread_Task( void );
		static CConcurrencyManager *Get_Concurrency_Manager( void );

	private:

		// Thread Local storage indices
		static uint32 ThreadHandle;
		static uint32 ConcurrencyManagerHandle;

		static bool Initialized;
};

#endif // THREAD_STATICS_H