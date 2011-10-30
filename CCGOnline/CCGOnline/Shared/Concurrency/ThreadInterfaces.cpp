/**********************************************************************************************************************

	ThreadInterfaces.cpp
		A component definining the read-only and write-only interfaces that a thread has

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

