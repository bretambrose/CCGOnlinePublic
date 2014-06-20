/**********************************************************************************************************************

	ProcessMailbox.cpp
		A component definining the class that holds both the read and write interfaces of a process mailbox.  Only the
		manager has access to this.

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

#include "ProcessMailbox.h"

#include "MailboxInterfaces.h"
#include "Containers/TBBConcurrentQueue.h"
#include "Containers/LockingConcurrentQueue.h"
#include "ProcessMessageFrame.h"

/**********************************************************************************************************************
	CProcessMailbox::CProcessMailbox -- constructor

		process_id -- id of the process these interfaces correspond to
		properties -- properties of the owning process
					
**********************************************************************************************************************/
CProcessMailbox::CProcessMailbox( EProcessID::Enum process_id, const SProcessProperties &properties ) :
	ProcessID( process_id ),
	Properties( properties ),
	WriteOnlyMailbox( nullptr ),
	ReadOnlyMailbox( nullptr )
{
	shared_ptr< IConcurrentQueue< unique_ptr< CProcessMessageFrame > > > queue = static_pointer_cast< IConcurrentQueue< unique_ptr< CProcessMessageFrame > > >( std::make_shared< ProcessToProcessQueueType >() );

	WriteOnlyMailbox.reset( new CWriteOnlyMailbox( process_id, properties, queue ) );
	ReadOnlyMailbox.reset( new CReadOnlyMailbox( queue ) );
}

/**********************************************************************************************************************
	CProcessMailbox::~CProcessMailbox -- destructor
					
**********************************************************************************************************************/
CProcessMailbox::~CProcessMailbox()
{
}

