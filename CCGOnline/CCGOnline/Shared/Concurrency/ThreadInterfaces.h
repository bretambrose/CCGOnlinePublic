/**********************************************************************************************************************

	[Placeholder for eventual source license]

	ThreadInterfaces.h
		A component definining the read-only and write-only interfaces that a thread has

	(c) Copyright 2011, Bret Ambrose.  All rights reserved.

**********************************************************************************************************************/

#ifndef THREAD_INTERFACES_H
#define THREAD_INTERFACES_H

#include "ThreadKey.h"

class CThreadMessageFrame;
template < typename T > class IConcurrentQueueBase;

// The write-only interface to a thread task.  Other threads talk to a thread by writing messages to this
class CWriteOnlyThreadInterface
{
	public:

		CWriteOnlyThreadInterface( const SThreadKey &target_key, const shared_ptr< IConcurrentQueueBase< shared_ptr< CThreadMessageFrame > > > &write_queue );
		~CWriteOnlyThreadInterface();

		const SThreadKey &Get_Target_Key( void ) const { return TargetKey; }

		void Add_Frame( const shared_ptr< CThreadMessageFrame > &frame );

	private:

		SThreadKey TargetKey;

		shared_ptr< IConcurrentQueueBase< shared_ptr< CThreadMessageFrame > > > WriteQueue;

};

// The read-only interface to a thread task.  A thread task processes messages by reading from this
class CReadOnlyThreadInterface
{
	public:

		CReadOnlyThreadInterface( const shared_ptr< IConcurrentQueueBase< shared_ptr< CThreadMessageFrame > > > &read_queue );
		~CReadOnlyThreadInterface();

		void Remove_Frames( std::vector< shared_ptr< CThreadMessageFrame > > &frames );

	private:

		shared_ptr< IConcurrentQueueBase< shared_ptr< CThreadMessageFrame > > > ReadQueue;

};

#endif // THREAD_INTERFACES_H