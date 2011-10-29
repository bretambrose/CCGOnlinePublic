/**********************************************************************************************************************

	[Placeholder for eventual source license]

	ThreadMessageHandlerBase.h
		A component defining a bare base class for objects that handle thread messages.  In practice these objects
		are simple trampolines that forward handling to a delegate while retaining type safety and
		generic behavior.

	(c) Copyright 2011, Bret Ambrose.  All rights reserved.

**********************************************************************************************************************/

#ifndef THREAD_MESSAGE_HANDLER_BASE_H
#define THREAD_MESSAGE_HANDLER_BASE_H

#include "MessageHandler.h"

struct SThreadKey;
class IThreadMessage;

// A simple base class for all thread message handlers
class IThreadMessageHandler : public IMessageHandler< SThreadKey, IThreadMessage >
{
	public:

		typedef IMessageHandler< SThreadKey, IThreadMessage > BASECLASS;

		IThreadMessageHandler( void ) :
			BASECLASS()
		{}

		virtual ~IThreadMessageHandler() {}

};

#endif // THREAD_MESSAGE_HANDLER_BASE_H
