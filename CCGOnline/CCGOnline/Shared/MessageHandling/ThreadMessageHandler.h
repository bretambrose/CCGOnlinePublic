/**********************************************************************************************************************

	ThreadMessageHandler.h
		A component defining a generic handler for thread message objects.  Implemented as a simple trampoline that 
		forwards handling to a delegate while retaining type safety and generic behavior.

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

#ifndef THREAD_MESSAGE_HANDLER_H
#define THREAD_MESSAGE_HANDLER_H

#include "ThreadMessageHandlerBase.h"

// Generic handler for thread messages
template< typename MessageType >
class TThreadMessageHandler : public IThreadMessageHandler
{
	public:

		// signature of the actual handling function for a specific message type
		typedef FastDelegate2< const SThreadKey &, const shared_ptr< const MessageType > &, void > HandlerFunctorType;

		typedef IThreadMessageHandler BASECLASS;

		TThreadMessageHandler( void ) :
			BASECLASS(),
			MessageHandler()
		{}

		TThreadMessageHandler( const TThreadMessageHandler &rhs ) :
			BASECLASS(),
			MessageHandler( rhs.MessageHandler )
		{}

		TThreadMessageHandler( const HandlerFunctorType &message_handler ) :
			BASECLASS(),
			MessageHandler( message_handler )
		{}

		virtual void Handle_Message( const SThreadKey &source, const shared_ptr< const IThreadMessage > &message ) const
		{
			// The handlers are tracked generically so they can go in one big hash table, but our forwarding delegates
			// have specific type signatures, requiring a down cast that preserves smart pointer reference counting
			shared_ptr< const MessageType > down_cast_message = static_pointer_cast< const MessageType >( message );
			MessageHandler( source, down_cast_message );
		}

	private:

		HandlerFunctorType MessageHandler;
};

// Utility macro for handler registration
#define REGISTER_THIS_HANDLER( x, y, z ) Register_Handler( typeid( x ), shared_ptr< IThreadMessageHandler >( new TThreadMessageHandler< x >( TThreadMessageHandler< x >::HandlerFunctorType( this, &y::z ) ) ) );

#endif // THREAD_MESSAGE_HANDLER_H
