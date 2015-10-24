/**********************************************************************************************************************

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

#pragma once

#include "ProcessMessageHandlerBase.h"

namespace IP
{
namespace Execution
{

enum class EProcessID;

namespace Messaging
{

// Generic handler for thread messages
template< typename MessageType >
class TProcessMessageHandler : public IProcessMessageHandler
{
	public:

		// signature of the actual handling function for a specific message type
		using HandlerFunctorType = FastDelegate2< IP::Execution::EProcessID, std::unique_ptr< const MessageType > &, void >;

		using BASECLASS = IProcessMessageHandler;

		TProcessMessageHandler( void ) :
			BASECLASS(),
			MessageHandler()
		{}

		TProcessMessageHandler( const TProcessMessageHandler< MessageType > &rhs ) :
			BASECLASS(),
			MessageHandler( rhs.MessageHandler )
		{}

		TProcessMessageHandler( const HandlerFunctorType &message_handler ) :
			BASECLASS(),
			MessageHandler( message_handler )
		{}

		virtual void Handle_Message( EProcessID source_process_id, std::unique_ptr< const IProcessMessage > &message ) const override
		{
			// The handlers are tracked generically so they can go in one big hash table, but our forwarding delegates
			// have specific type signatures, requiring a down cast that preserves smart pointer reference counting
			const MessageType *raw_msg = static_cast< const MessageType * >( message.release() );

			std::unique_ptr< const MessageType > down_cast_message( raw_msg );
			MessageHandler( source_process_id, down_cast_message );
		}

	private:

		HandlerFunctorType MessageHandler;
};

template < typename T, typename U >
void Register_This_Handler( U* registry, void (U::*handler_function)( IP::Execution::EProcessID, std::unique_ptr< const T > & ) )
{
	std::unique_ptr< IProcessMessageHandler > handler( new TProcessMessageHandler< T >( TProcessMessageHandler< T >::HandlerFunctorType( registry, handler_function ) ) );
	registry->Register_Handler( typeid( T ), handler );
}

} // namespace Messaging
} // namespace Execution
} // namespace IP

// Utility macro for handler registration
#define REGISTER_THIS_HANDLER( x, y, z ) IP::Execution::Messaging::Register_This_Handler< x, y >( this, &y::z );


