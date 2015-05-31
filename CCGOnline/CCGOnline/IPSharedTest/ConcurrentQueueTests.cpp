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

#include "stdafx.h"

#include "IPShared/Concurrency/Containers/LockingConcurrentQueue.h"
#include "IPShared/Concurrency/Containers/TBBConcurrentQueue.h"
#include "IPShared/Concurrency/Messaging/LoggingMessages.h"
#include "IPShared/Concurrency/ProcessConstants.h"

TEST( ConcurrentQueueTests, Add_Remove_Locking )
{
	IConcurrentQueue< int > *int_queue = new CLockingConcurrentQueue< int >();

	int_queue->Move_Item( 5 );
	int_queue->Move_Item( 10 );
	int_queue->Move_Item( 15 );

	std::vector< int > items;
	int_queue->Remove_Items( items );

	ASSERT_TRUE( items[ 0 ] == 5 );
	ASSERT_TRUE( items[ 1 ] == 10 );
	ASSERT_TRUE( items[ 2 ] == 15 );

	delete int_queue;
}

TEST( ConcurrentQueueTests, Add_Remove_Lockless )
{
	IConcurrentQueue< int > *int_queue = new CTBBConcurrentQueue< int >();

	int_queue->Move_Item( 5 );
	int_queue->Move_Item( 10 );
	int_queue->Move_Item( 15 );

	std::vector< int > items;
	int_queue->Remove_Items( items );

	ASSERT_TRUE( items[ 0 ] == 5 );
	ASSERT_TRUE( items[ 1 ] == 10 );
	ASSERT_TRUE( items[ 2 ] == 15 );

	delete int_queue;
}

static const std::wstring LOG_MESSAGE_1( L"Log Message 1" );
static const std::wstring LOG_MESSAGE_2( L"Log Message 2" );

TEST( ConcurrentQueueTests, Add_Remove_Shared_Ptr_Locking )
{
	IConcurrentQueue< unique_ptr< const CLogRequestMessage > > *message_queue = new CLockingConcurrentQueue< unique_ptr< const CLogRequestMessage > >();

	unique_ptr< const CLogRequestMessage > message1( new CLogRequestMessage( MANAGER_PROCESS_PROPERTIES, LOG_MESSAGE_1 ) );
	message_queue->Move_Item( std::move( message1 ) );

	unique_ptr< const CLogRequestMessage > message2( new CLogRequestMessage( MANAGER_PROCESS_PROPERTIES, LOG_MESSAGE_2 ) );
	message_queue->Move_Item( std::move( message2 ) );

	std::vector< unique_ptr< const CLogRequestMessage > > items;
	message_queue->Remove_Items( items );

	ASSERT_TRUE( items[ 0 ]->Get_Message() == LOG_MESSAGE_1 );
	ASSERT_TRUE( items[ 0 ]->Get_Source_Properties() == MANAGER_PROCESS_PROPERTIES );
	ASSERT_TRUE( items[ 1 ]->Get_Message() == LOG_MESSAGE_2 );
	ASSERT_TRUE( items[ 1 ]->Get_Source_Properties() == MANAGER_PROCESS_PROPERTIES );

	delete message_queue;
	message_queue = nullptr;
}

TEST( ConcurrentQueueTests, Add_Remove_Shared_Ptr_Lockless )
{
	IConcurrentQueue< unique_ptr< const CLogRequestMessage > > *message_queue = new CTBBConcurrentQueue< unique_ptr< const CLogRequestMessage > >();

	unique_ptr< const CLogRequestMessage > message1( new CLogRequestMessage( MANAGER_PROCESS_PROPERTIES, LOG_MESSAGE_1 ) );
	message_queue->Move_Item( std::move( message1 ) );

	unique_ptr< const CLogRequestMessage > message2( new CLogRequestMessage( MANAGER_PROCESS_PROPERTIES, LOG_MESSAGE_2 ) );
	message_queue->Move_Item( std::move( message2 ) );

	std::vector< unique_ptr< const CLogRequestMessage > > items;
	message_queue->Remove_Items( items );

	ASSERT_TRUE( items[ 0 ]->Get_Message() == LOG_MESSAGE_1 );
	ASSERT_TRUE( items[ 0 ]->Get_Source_Properties() == MANAGER_PROCESS_PROPERTIES );
	ASSERT_TRUE( items[ 1 ]->Get_Message() == LOG_MESSAGE_2 );
	ASSERT_TRUE( items[ 1 ]->Get_Source_Properties() == MANAGER_PROCESS_PROPERTIES );

	delete message_queue;
	message_queue = nullptr;
}