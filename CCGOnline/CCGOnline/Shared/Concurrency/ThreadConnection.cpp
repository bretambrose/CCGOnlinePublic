/**********************************************************************************************************************

	[Placeholder for eventual source license]

	ThreadConnection.cpp
		A component definining the class that holds both the read and write interfaces of a thread task.  Only the
		manager has access to this.

	(c) Copyright 2011, Bret Ambrose.  All rights reserved.

**********************************************************************************************************************/

#include "stdafx.h"

#include "ThreadConnection.h"

#include "ThreadInterfaces.h"
#include "Concurrency/Containers/ConcurrentQueue.h"
#include "Concurrency/Containers/ConcurrentQueueLocking.h"
#include "ThreadMessageFrame.h"

// Controls which concurrent queue implementation we use
typedef CConcurrentQueue< shared_ptr< CThreadMessageFrame > > ThreadToThreadQueueType;

/**********************************************************************************************************************
	CThreadConnection::CThreadConnection -- constructor

		key -- thread key of the thread these interfaces correspond to
					
**********************************************************************************************************************/
CThreadConnection::CThreadConnection( const SThreadKey &key ) :
	Key( key ),
	Queue( new ThreadToThreadQueueType ),
	WriterInterface( new CWriteOnlyThreadInterface( key, Queue ) ),
	ReaderInterface( new CReadOnlyThreadInterface( Queue ) )
{
}

/**********************************************************************************************************************
	CThreadConnection::~CThreadConnection -- destructor
					
**********************************************************************************************************************/
CThreadConnection::~CThreadConnection()
{
}


