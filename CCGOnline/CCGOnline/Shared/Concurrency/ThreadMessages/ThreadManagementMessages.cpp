/**********************************************************************************************************************

	ThreadManagementMessages.h
		A component containing definitions for thread messages that manage and/or control thread tasks

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

#include "ThreadManagementMessages.h"

#include "Concurrency\ThreadTaskInterface.h"

/**********************************************************************************************************************
	CAddThreadMessage::CAddThreadMessage -- constructor
	
		thread_task -- the thread to add to the conurrency system
		return_interface -- should the manager return an interface to this new thread to the requesting thread?
		forward_creator_interface -- should the manager forward the requesting thread's interface to the new thread?
		
**********************************************************************************************************************/
CAddThreadMessage::CAddThreadMessage( const shared_ptr< IThreadTask > &thread_task, bool return_interface, bool forward_creator_interface ) :
	ThreadTask( thread_task ),
	ReturnInterface( return_interface ),
	ForwardCreatorInterface( forward_creator_interface )
{
}

/**********************************************************************************************************************
	CAddThreadMessage::~CAddThreadMessage -- destructor
		
**********************************************************************************************************************/
CAddThreadMessage::~CAddThreadMessage()
{
}

