/**********************************************************************************************************************

	MailboxInterfaces.cpp
		A component definining the read-only and write-only mailbox interfaces that a virtual process has

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

#include "MailboxInterfaces.h"

#include "Concurrency/Containers/ConcurrentQueueInterface.h"
#include "VirtualProcessMessageFrame.h"

/**********************************************************************************************************************
	CWriteOnlyMailbox::CWriteOnlyMailbox -- constructor

		target_key -- the key of thread that this write-only interface refers to
		write_queue -- the concurrency queue containing message frames targeted for the corresponding virtual process
					
**********************************************************************************************************************/
CWriteOnlyMailbox::CWriteOnlyMailbox( const SThreadKey &target_key, 
												  const shared_ptr< IConcurrentQueue< shared_ptr< CVirtualProcessMessageFrame > > > &write_queue ) :
	TargetKey( target_key ),
	WriteQueue( write_queue )
{
}

/**********************************************************************************************************************
	CWriteOnlyMailbox::~CWriteOnlyMailbox -- destructor
					
**********************************************************************************************************************/
CWriteOnlyMailbox::~CWriteOnlyMailbox()
{
}

/**********************************************************************************************************************
	CWriteOnlyMailbox::Add_Frame -- adds a message frame to this mailbox's write queue

		frame -- the message frame to add
					
**********************************************************************************************************************/
void CWriteOnlyMailbox::Add_Frame( const shared_ptr< CVirtualProcessMessageFrame > &frame )
{
	FATAL_ASSERT( frame.get() != nullptr );

	WriteQueue->Add_Item( frame );
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/**********************************************************************************************************************
	CReadOnlyMailbox::CReadOnlyMailbox -- constructor

		read_queue -- the concurrent queue of message frames that will get read from
					
**********************************************************************************************************************/
CReadOnlyMailbox::CReadOnlyMailbox( const shared_ptr< IConcurrentQueue< shared_ptr< CVirtualProcessMessageFrame > > > &read_queue ) :
	ReadQueue( read_queue )
{
	FATAL_ASSERT( ReadQueue.get() != nullptr );
}

/**********************************************************************************************************************
	CReadOnlyMailbox::~CReadOnlyMailbox -- destructor
					
**********************************************************************************************************************/
CReadOnlyMailbox::~CReadOnlyMailbox()
{
}

/**********************************************************************************************************************
	CReadOnlyMailbox::Remove_Frames -- removes all frames from the read queue and adds them to an output
		parameter vector

		frames -- output parameter for all the frames currently on the read queue
					
**********************************************************************************************************************/
void CReadOnlyMailbox::Remove_Frames( std::vector< shared_ptr< CVirtualProcessMessageFrame > > &frames )
{
	ReadQueue->Remove_Items( frames );
}

