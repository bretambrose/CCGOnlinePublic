/**********************************************************************************************************************

	[Placeholder for eventual source license]

	ConcurrencyQueueTests.cpp
		defines unit tests for concurrency queue functionality

	(c) Copyright 2011, Bret Ambrose.  All rights reserved.

**********************************************************************************************************************/

#include "stdafx.h"

#include "Concurrency/Containers/ConcurrentQueue.h"
#include "Concurrency/ThreadMessages/LoggingMessages.h"
#include "Concurrency/ThreadConstants.h"

TEST( ConcurrentQueueTests, Add_Remove )
{
	IConcurrentQueueBase< int > *int_queue = new CConcurrentQueue< int >();

	int_queue->Add_Item( 5 );
	int_queue->Add_Item( 10 );
	int_queue->Add_Item( 15 );

	std::vector< int > items;
	int_queue->Remove_Items( items );

	ASSERT_TRUE( items[ 0 ] == 5 );
	ASSERT_TRUE( items[ 1 ] == 10 );
	ASSERT_TRUE( items[ 2 ] == 15 );

	delete int_queue;
}

static const std::wstring LOG_MESSAGE_1( L"Log Message 1" );
static const std::wstring LOG_MESSAGE_2( L"Log Message 2" );

TEST( ConcurrentQueueTests, Add_Remove_Shared_Ptr )
{
	IConcurrentQueueBase< shared_ptr< const CLogRequestMessage > > *message_queue = new CConcurrentQueue< shared_ptr< const CLogRequestMessage > >();

	{
		shared_ptr< const CLogRequestMessage > message1( new CLogRequestMessage( MANAGER_THREAD_KEY, LOG_MESSAGE_1 ) );
		message_queue->Add_Item( message1 );
	}

	message_queue->Add_Item( shared_ptr< const CLogRequestMessage >( new CLogRequestMessage( MANAGER_THREAD_KEY, LOG_MESSAGE_2 ) ) );

	std::vector< shared_ptr< const CLogRequestMessage > > items;
	message_queue->Remove_Items( items );

	ASSERT_TRUE( items[ 0 ]->Get_Message() == LOG_MESSAGE_1 );
	ASSERT_TRUE( items[ 0 ]->Get_Source_Key() == MANAGER_THREAD_KEY );
	ASSERT_TRUE( items[ 1 ]->Get_Message() == LOG_MESSAGE_2 );
	ASSERT_TRUE( items[ 1 ]->Get_Source_Key() == MANAGER_THREAD_KEY );

	delete message_queue;
	message_queue = NULL;
}