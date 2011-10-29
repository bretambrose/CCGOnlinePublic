/**********************************************************************************************************************

	[Placeholder for eventual source license]

	ThreadMessageFrame.cpp
		A component definining a container of thread messages.  Batching messages into a container leads to more
		efficiency with the concurrency queues in high-traffic situations.

	(c) Copyright 2011, Bret Ambrose.  All rights reserved.

**********************************************************************************************************************/

#include "stdafx.h"

#include "ThreadMessageFrame.h"

#include "ThreadMessages/ThreadMessage.h"

/**********************************************************************************************************************
	CThreadMessageFrame::~CThreadMessageFrame -- destructor, defined internally to avoid header dependency on
		IThreadMessage
					
**********************************************************************************************************************/
CThreadMessageFrame::~CThreadMessageFrame()
{
}

/**********************************************************************************************************************
	CThreadMessageFrame::Add_Message -- adds a message to the container

		message -- message to add to the frame
					
**********************************************************************************************************************/
void CThreadMessageFrame::Add_Message( const shared_ptr< const IThreadMessage > &message )
{
	Messages.push_back( message );
}

