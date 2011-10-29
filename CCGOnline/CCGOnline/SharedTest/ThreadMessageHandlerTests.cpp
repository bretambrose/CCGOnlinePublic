/**********************************************************************************************************************

	[Placeholder for eventual source license]

	ThreadMessageHandlerTests.cpp
		defines unit tests for thread message handler related functionality

	(c) Copyright 2011, Bret Ambrose.  All rights reserved.

**********************************************************************************************************************/

#include "stdafx.h"

#include "MessageHandling/ThreadMessageHandler.h"
#include "Concurrency/ThreadMessages/LoggingMessages.h"
#include "Concurrency/ThreadMessages/ThreadManagementMessages.h"
#include "Concurrency/ThreadConstants.h"
#include "TypeInfoUtils.h"

class CMockMessageHandlerTracker
{
	public:

		CMockMessageHandlerTracker( void ) :
			MessageHandlers(),
			LastLogMessage( L"" ),
			LastShutdownKey( 0 )
		{}

		~CMockMessageHandlerTracker()
		{
			MessageHandlers.clear();
		}

		void Register_Handlers( void )
		{
			REGISTER_THIS_HANDLER( CLogRequestMessage, CMockMessageHandlerTracker, Handle_Log_Request );
			REGISTER_THIS_HANDLER( CShutdownThreadMessage, CMockMessageHandlerTracker, Handle_Shutdown_Request );
		}

		void Handle_Message( const SThreadKey &key, const shared_ptr< const IThreadMessage > &message )
		{
			const IThreadMessage *msg_base = message.get();

			Loki::TypeInfo hash_key( typeid( *msg_base ) );
			auto iter = MessageHandlers.find( hash_key );
			FATAL_ASSERT( iter != MessageHandlers.end() );

			iter->second->Handle_Message( key, message );
		}

		const std::wstring &Get_Last_Log_Message( void ) const { return LastLogMessage; }
		const SThreadKey &Get_Last_Shutdown_Key( void ) const { return LastShutdownKey; }

	private:

		void Register_Handler( const std::type_info &message_type_info, const shared_ptr< IThreadMessageHandler > &handler )
		{
			Loki::TypeInfo key( message_type_info );

			ASSERT_TRUE( MessageHandlers.find( key ) == MessageHandlers.end() );
			MessageHandlers[ key ] = handler;
		}

		void Handle_Log_Request( const SThreadKey & /*key*/, const shared_ptr< const CLogRequestMessage > &message )
		{
			LastLogMessage = message->Get_Message();
		}

		void Handle_Shutdown_Request( const SThreadKey & /*key*/, const shared_ptr< const CShutdownThreadMessage > &message )
		{
			LastShutdownKey = message->Get_Key();
		}

		stdext::hash_map< Loki::TypeInfo, shared_ptr< IThreadMessageHandler >, STypeInfoContainerHelper > MessageHandlers;

		std::wstring LastLogMessage;

		SThreadKey LastShutdownKey;
};

static const std::wstring LOG_MESSAGE_1( L"Message Handler Test 1" );
static const std::wstring LOG_MESSAGE_2( L"Aoooooooga" );

TEST( ThreadMessageHandlerTests, Register_And_Handle )
{
	CMockMessageHandlerTracker tracker;
	tracker.Register_Handlers();

	tracker.Handle_Message( MANAGER_THREAD_KEY, shared_ptr< const IThreadMessage >( new CLogRequestMessage( MANAGER_THREAD_KEY, LOG_MESSAGE_1 ) ) );
	ASSERT_TRUE( tracker.Get_Last_Log_Message() == LOG_MESSAGE_1 );

	tracker.Handle_Message( MANAGER_THREAD_KEY, shared_ptr< const IThreadMessage >( new CLogRequestMessage( MANAGER_THREAD_KEY, LOG_MESSAGE_2 ) ) );
	ASSERT_TRUE( tracker.Get_Last_Log_Message() == LOG_MESSAGE_2 );

	tracker.Handle_Message( MANAGER_THREAD_KEY, shared_ptr< const IThreadMessage >( new CShutdownThreadMessage( LOG_THREAD_KEY ) ) );
	ASSERT_TRUE( tracker.Get_Last_Shutdown_Key() == LOG_THREAD_KEY );

	tracker.Handle_Message( MANAGER_THREAD_KEY, shared_ptr< const IThreadMessage >( new CShutdownThreadMessage( MANAGER_THREAD_KEY ) ) );
	ASSERT_TRUE( tracker.Get_Last_Shutdown_Key() == MANAGER_THREAD_KEY );
	
}