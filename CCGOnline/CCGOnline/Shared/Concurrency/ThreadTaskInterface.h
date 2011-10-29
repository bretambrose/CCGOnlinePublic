/**********************************************************************************************************************

	[Placeholder for eventual source license]

	ThreadTaskInterface.h
		A component defining the top-level pure virtual interface to a thread task.

	(c) Copyright 2011, Bret Ambrose.  All rights reserved.

**********************************************************************************************************************/

#ifndef THREAD_TASK_INTERFACE_H
#define THREAD_TASK_INTERFACE_H

class CTaskScheduler;
class IThreadMessage;

struct SThreadKey;

// Pure virtual interface for all thread tasks
class IThreadTask
{
	public:
		
		IThreadTask( void ) {}
		virtual ~IThreadTask() {}

		virtual void Initialize( void ) = 0;

		virtual const SThreadKey &Get_Key( void ) const = 0;

		virtual void Send_Thread_Message( const SThreadKey &dest_key, const shared_ptr< const IThreadMessage > &message ) = 0;
		virtual void Log( const std::wstring &message ) = 0;

		virtual CTaskScheduler *Get_Task_Scheduler( void ) const = 0;

		virtual void Flush_Partitioned_Messages( void ) = 0;
};

#endif // THREAD_TASK_INTERFACE_H
