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

#include "stdafx.h"

#include "MailboxInterfaces.h"

#include "IPShared/Concurrency/Containers/ConcurrentQueueInterface.h"
#include "ProcessMessageFrame.h"


CWriteOnlyMailbox::CWriteOnlyMailbox( EProcessID::Enum process_id, 
												  const SProcessProperties &properties, 
												  const shared_ptr< IConcurrentQueue< unique_ptr< CProcessMessageFrame > > > &write_queue ) :
	ProcessID( process_id ),
	Properties( properties ),
	WriteQueue( write_queue )
{
	FATAL_ASSERT( WriteQueue.get() != nullptr );
}


CWriteOnlyMailbox::~CWriteOnlyMailbox()
{
}


void CWriteOnlyMailbox::Add_Frame( unique_ptr< CProcessMessageFrame > &frame )
{
	FATAL_ASSERT( frame.get() != nullptr );

	WriteQueue->Move_Item( std::move( frame ) );
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


CReadOnlyMailbox::CReadOnlyMailbox( const shared_ptr< IConcurrentQueue< unique_ptr< CProcessMessageFrame > > > &read_queue ) :
	ReadQueue( read_queue )
{
	FATAL_ASSERT( ReadQueue.get() != nullptr );
}


CReadOnlyMailbox::~CReadOnlyMailbox()
{
}


void CReadOnlyMailbox::Remove_Frames( std::vector< unique_ptr< CProcessMessageFrame > > &frames )
{
	ReadQueue->Remove_Items( frames );
}

