/**********************************************************************************************************************

	[Placeholder for eventual source license]

	ThreadManagementMessages.h
		A component containing definitions for thread messages that manage and/or control thread tasks

	(c) Copyright 2011, Bret Ambrose.  All rights reserved.

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

