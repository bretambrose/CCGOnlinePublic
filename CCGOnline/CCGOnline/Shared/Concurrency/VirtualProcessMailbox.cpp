/**********************************************************************************************************************

	VirtualProcessMailbox.cpp
		A component definining the class that holds both the read and write interfaces of a virtual process mailbox.  Only the
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

#include "VirtualProcessMailbox.h"

#include "MailboxInterfaces.h"
#include "Concurrency/Containers/TBBConcurrentQueue.h"
#include "VirtualProcessMessageFrame.h"

// Controls which concurrent queue implementation we use
typedef CTBBConcurrentQueue< shared_ptr< CVirtualProcessMessageFrame > > ProcessToProcessQueueType;

/**********************************************************************************************************************
	CVirtualProcessMailbox::CVirtualProcessMailbox -- constructor

		key -- thread key of the thread these interfaces correspond to
					
**********************************************************************************************************************/
CVirtualProcessMailbox::CVirtualProcessMailbox( const SThreadKey &key ) :
	Key( key ),
	Queue( new ProcessToProcessQueueType ),
	WriteOnlyMailbox( new CWriteOnlyMailbox( key, Queue ) ),
	ReadOnlyMailbox( new CReadOnlyMailbox( Queue ) )
{
}

/**********************************************************************************************************************
	CVirtualProcessMailbox::~CVirtualProcessMailbox -- destructor
					
**********************************************************************************************************************/
CVirtualProcessMailbox::~CVirtualProcessMailbox()
{
}


