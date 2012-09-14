/**********************************************************************************************************************

	VirtualProcessMessageHandler.h
		A component defining a generic handler for virtual process message objects.  Implemented as a simple trampoline that 
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

#ifndef VIRTUAL_PROCESS_MESSAGE_HANDLER_H
#define VIRTUAL_PROCESS_MESSAGE_HANDLER_H

#include "VirtualProcessMessageHandlerBase.h"

// Generic handler for thread messages
template< typename MessageType >
class TVirtualProcessMessageHandler : public IVirtualProcessMessageHandler
{
	public:

		// signature of the actual handling function for a specific message type
		typedef FastDelegate2< EVirtualProcessID::Enum, const shared_ptr< const MessageType > &, void > HandlerFunctorType;

		typedef IVirtualProcessMessageHandler BASECLASS;

		TVirtualProcessMessageHandler( void ) :
			BASECLASS(),
			MessageHandler()
		{}

		TVirtualProcessMessageHandler( const TVirtualProcessMessageHandler &rhs ) :
			BASECLASS(),
			MessageHandler( rhs.MessageHandler )
		{}

		TVirtualProcessMessageHandler( const HandlerFunctorType &message_handler ) :
			BASECLASS(),
			MessageHandler( message_handler )
		{}

		virtual void Handle_Message( EVirtualProcessID::Enum source_process_id, const shared_ptr< const IVirtualProcessMessage > &message ) const
		{
			// The handlers are tracked generically so they can go in one big hash table, but our forwarding delegates
			// have specific type signatures, requiring a down cast that preserves smart pointer reference counting
			shared_ptr< const MessageType > down_cast_message = static_pointer_cast< const MessageType >( message );
			MessageHandler( source_process_id, down_cast_message );
		}

	private:

		HandlerFunctorType MessageHandler;
};

// Utility macro for handler registration
#define REGISTER_THIS_HANDLER( x, y, z ) Register_Handler( typeid( x ), shared_ptr< IVirtualProcessMessageHandler >( new TVirtualProcessMessageHandler< x >( TVirtualProcessMessageHandler< x >::HandlerFunctorType( this, &y::z ) ) ) );

#endif // VIRTUAL_PROCESS_MESSAGE_HANDLER_H
