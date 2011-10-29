/**********************************************************************************************************************

	[Placeholder for eventual source license]

	MessageHandler.h
		A component defining a top-level base class for objects that handle a set of related message classes.  In 
		practice these objects are simple trampolines that forward handling to a delegate while retaining type safety 
		and generic behavior.

	(c) Copyright 2011, Bret Ambrose.  All rights reserved.

**********************************************************************************************************************/

#ifndef MESSAGE_HANDLER_H
#define MESSAGE_HANDLER_H

// A simple base class for all message handlers
template< typename MessageSourceType, typename MessageBaseType >
class IMessageHandler
{
	public:

		IMessageHandler( void ) {}
		virtual ~IMessageHandler() {}

		virtual void Handle_Message( const MessageSourceType &source, const shared_ptr< const MessageBaseType > &message ) const = 0;
};

#endif // MESSAGE_HANDLER_H
