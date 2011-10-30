/**********************************************************************************************************************

	ThreadConnection.cpp
		A component definining the class that holds both the read and write interfaces of a thread task.  Only the
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


