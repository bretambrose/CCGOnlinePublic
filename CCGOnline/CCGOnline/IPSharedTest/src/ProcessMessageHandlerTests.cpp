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

#include <IPCore/Debug/DebugAssert.h>
#include <IPCore/Memory/Stl/UnorderedMap.h>
#include <IPShared/MessageHandling/ProcessMessageHandler.h>
#include <IPShared/Concurrency/Messaging/LoggingMessages.h>
#include <IPShared/Concurrency/Messaging/ProcessManagementMessages.h>
#include <IPShared/Concurrency/ProcessConstants.h>
#include <IPShared/Concurrency/ProcessID.h>
#include <IPShared/TypeInfoUtils.h>
#include <gtest/gtest.h>
#include <loki/LokiTypeInfo.h>

using namespace IP;
using namespace IP::Execution;
using namespace IP::Execution::Messaging;

class CMockMessageHandlerTracker
{
	public:

		CMockMessageHandlerTracker( void ) :
			MessageHandlers(),
			LastLogMessage( "" ),
			LastShutdownProcessID( EProcessID::INVALID )
		{}

		~CMockMessageHandlerTracker()
		{
			MessageHandlers.clear();
		}

		void Register_Handlers( void )
		{
			REGISTER_THIS_HANDLER( CLogRequestMessage, CMockMessageHandlerTracker, Handle_Log_Request );
			REGISTER_THIS_HANDLER( CShutdownProcessMessage, CMockMessageHandlerTracker, Handle_Shutdown_Process_Request );
		}

		const IP::String &Get_Last_Log_Message( void ) const { return LastLogMessage; }
		EProcessID Get_Last_Shutdown_Process_ID( void ) const { return LastShutdownProcessID; }

		void Register_Handler( const std::type_info &message_type_info, IP::UniquePtr< IProcessMessageHandler > &handler )
		{
			Loki::TypeInfo key( message_type_info );

			ASSERT_TRUE( MessageHandlers.find( key ) == MessageHandlers.end() );
			MessageHandlers.insert( MessageHandlerTableType::value_type( key, std::move( handler ) ) );
		}

		void Handle_Message( EProcessID process_id, IP::UniquePtr< const IProcessMessage > &message )
		{
			const IProcessMessage *msg_base = message.get();

			Loki::TypeInfo hash_key( typeid( *msg_base ) );
			auto iter = MessageHandlers.find( hash_key );
			FATAL_ASSERT( iter != MessageHandlers.end() );

			iter->second->Handle_Message( process_id, message );
		}

	private:

		void Handle_Log_Request( EProcessID /*process_id*/, IP::UniquePtr< const CLogRequestMessage > &message )
		{
			LastLogMessage = message->Get_Message();
		}

		void Handle_Shutdown_Process_Request( EProcessID /*process_id*/, IP::UniquePtr< const CShutdownProcessMessage > &message )
		{
			LastShutdownProcessID = message->Get_Process_ID();
		}

		using MessageHandlerTableType = IP::UnorderedMap< Loki::TypeInfo, IP::UniquePtr< IProcessMessageHandler >, STypeInfoContainerHelper >;
		MessageHandlerTableType MessageHandlers;

		IP::String LastLogMessage;

		EProcessID LastShutdownProcessID;
};

static const IP::String LOG_MESSAGE_1( "Message Handler Test 1" );
static const IP::String LOG_MESSAGE_2( "Aoooooooga" );

TEST( ProcessMessageHandlerTests, Register_And_Handle )
{
	CMockMessageHandlerTracker tracker;
	tracker.Register_Handlers();

	auto message1 = IP::Make_Const_Process_Message< CLogRequestMessage >( MEMORY_TAG, MANAGER_PROCESS_PROPERTIES, LOG_MESSAGE_1 );
	tracker.Handle_Message( EProcessID::CONCURRENCY_MANAGER, message1 );
	ASSERT_TRUE( tracker.Get_Last_Log_Message() == LOG_MESSAGE_1 );

	auto message2 = IP::Make_Const_Process_Message< CLogRequestMessage >( MEMORY_TAG, LOGGING_PROCESS_PROPERTIES, LOG_MESSAGE_2 );
	tracker.Handle_Message( EProcessID::CONCURRENCY_MANAGER, message2 );
	ASSERT_TRUE( tracker.Get_Last_Log_Message() == LOG_MESSAGE_2 );

	auto message3 = IP::Make_Const_Process_Message< CShutdownProcessMessage >( MEMORY_TAG, EProcessID::LOGGING );
	tracker.Handle_Message( EProcessID::CONCURRENCY_MANAGER, message3 );
	ASSERT_TRUE( tracker.Get_Last_Shutdown_Process_ID() == EProcessID::LOGGING );

	auto message4 = IP::Make_Const_Process_Message< CShutdownProcessMessage >( MEMORY_TAG, EProcessID::CONCURRENCY_MANAGER );
	tracker.Handle_Message( EProcessID::CONCURRENCY_MANAGER, message4 );
	ASSERT_TRUE( tracker.Get_Last_Shutdown_Process_ID() == EProcessID::CONCURRENCY_MANAGER );
	
}