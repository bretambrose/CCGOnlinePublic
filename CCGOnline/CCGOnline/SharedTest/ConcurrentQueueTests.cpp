/**********************************************************************************************************************

	ConcurrencyQueueTests.cpp
		defines unit tests for concurrency queue functionality

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

#include "stdafx.h"

#include "Concurrency/Containers/TBBConcurrentQueue.h"
#include "Concurrency/Messaging/LoggingMessages.h"
#include "Concurrency/VirtualProcessConstants.h"

TEST( ConcurrentQueueTests, Add_Remove )
{
	IConcurrentQueue< int > *int_queue = new CTBBConcurrentQueue< int >();

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
	IConcurrentQueue< shared_ptr< const CLogRequestMessage > > *message_queue = new CTBBConcurrentQueue< shared_ptr< const CLogRequestMessage > >();

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
	message_queue = nullptr;
}