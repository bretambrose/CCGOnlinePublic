/**********************************************************************************************************************

	[Placeholder for eventual source license]

	ThreadInterfaces.cpp
		A component definining the read-only and write-only interfaces that a thread has

	(c) Copyright 2011, Bret Ambrose.  All rights reserved.

**********************************************************************************************************************/

#include "stdafx.h"

#include "ThreadInterfaces.h"

#include "Concurrency/Containers/ConcurrentQueueBase.h"
#include "ThreadMessageFrame.h"

/**********************************************************************************************************************
	CWriteOnlyThreadInterface::CWriteOnlyThreadInterface -- constructor

		target_key -- the key of thread that this write-only interface refers to
		write_queue -- the concurrency queue containing message frames targeted for the corresponding thread
					
**********************************************************************************************************************/
CWriteOnlyThreadInterface::CWriteOnlyThreadInterface( const SThreadKey &target_key, 
																		const shared_ptr< IConcurrentQueueBase< shared_ptr< CThreadMessageFrame > > > &write_queue ) :
	TargetKey( target_key ),
	WriteQueue( write_queue )
{
}

/**********************************************************************************************************************
	CWriteOnlyThreadInterface::~CWriteOnlyThreadInterface -- destructor
					
**********************************************************************************************************************/
CWriteOnlyThreadInterface::~CWriteOnlyThreadInterface()
{
}

/**********************************************************************************************************************
	CWriteOnlyThreadInterface::Add_Frame -- adds a message frame to this interfaces write queue

		frame -- the message frame to add
					
**********************************************************************************************************************/
void CWriteOnlyThreadInterface::Add_Frame( const shared_ptr< CThreadMessageFrame > &frame )
{
	FATAL_ASSERT( frame.get() != nullptr );

	WriteQueue->Add_Item( frame );
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/**********************************************************************************************************************
	CReadOnlyThreadInterface::CReadOnlyThreadInterface -- constructor

		read_queue -- the concurrent queue of message frames that will get read from
					
**********************************************************************************************************************/
CReadOnlyThreadInterface::CReadOnlyThreadInterface( const shared_ptr< IConcurrentQueueBase< shared_ptr< CThreadMessageFrame > > > &read_queue ) :
	ReadQueue( read_queue )
{
	FATAL_ASSERT( ReadQueue.get() != nullptr );
}

/**********************************************************************************************************************
	CReadOnlyThreadInterface::~CReadOnlyThreadInterface -- destructor
					
**********************************************************************************************************************/
CReadOnlyThreadInterface::~CReadOnlyThreadInterface()
{
}

/**********************************************************************************************************************
	CReadOnlyThreadInterface::Remove_Frames -- removes all frames from the read queue and adds them to an output
		parameter vector

		frames -- output parameter for all the frames currently on the read queue
					
**********************************************************************************************************************/
void CReadOnlyThreadInterface::Remove_Frames( std::vector< shared_ptr< CThreadMessageFrame > > &frames )
{
	ReadQueue->Remove_Items( frames );
}

