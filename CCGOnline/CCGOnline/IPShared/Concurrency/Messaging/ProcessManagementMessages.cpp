/**********************************************************************************************************************

	ProcessManagementMessages.h
		A component containing definitions for messages that manage and/or control processes

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

#include "ProcessManagementMessages.h"

#include "IPShared/Concurrency/ProcessInterface.h"

/**********************************************************************************************************************
	CAddNewProcessMessage::CAddNewProcessMessage -- constructor
	
		process -- the process to add to the conurrency system
		return_interface -- should the manager return an interface to this new process to the requesting thread?
		forward_creator_interface -- should the manager forward the requesting process's interface to the new process?
		
**********************************************************************************************************************/
CAddNewProcessMessage::CAddNewProcessMessage( const shared_ptr< IProcess > &process, bool return_mailbox, bool forward_creator_mailbox ) :
	Process( process ),
	ReturnMailbox( return_mailbox ),
	ForwardCreatorMailbox( forward_creator_mailbox )
{
}

/**********************************************************************************************************************
	CAddNewProcessMessage::~CAddNewProcessMessage -- destructor
		
**********************************************************************************************************************/
CAddNewProcessMessage::~CAddNewProcessMessage()
{
}

