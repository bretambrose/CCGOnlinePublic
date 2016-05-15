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

#include <IPShared/Concurrency/Containers/LockingConcurrentQueue.h>
#include <IPShared/Concurrency/Containers/TBBConcurrentQueue.h>
#include <IPShared/Concurrency/Messaging/LoggingMessages.h>
#include <IPShared/Concurrency/ProcessConstants.h>
#include <gtest/gtest.h>

using namespace IP::Concurrency;
using namespace IP::Execution;
using namespace IP::Execution::Messaging;

TEST( ConcurrentQueueTests, Add_Remove_Locking )
{
	auto int_queue = IP::Make_Unique< CLockingConcurrentQueue< int > >( MEMORY_TAG );

	int_queue->Move_Item( 5 );
	int_queue->Move_Item( 10 );
	int_queue->Move_Item( 15 );

	IP::Vector< int > items;
	int_queue->Remove_Items( items );

	ASSERT_TRUE( items[ 0 ] == 5 );
	ASSERT_TRUE( items[ 1 ] == 10 );
	ASSERT_TRUE( items[ 2 ] == 15 );
}

TEST( ConcurrentQueueTests, Add_Remove_Lockless )
{
	auto int_queue = IP::Make_Unique< CTBBConcurrentQueue< int > >( MEMORY_TAG );

	int_queue->Move_Item( 5 );
	int_queue->Move_Item( 10 );
	int_queue->Move_Item( 15 );

	IP::Vector< int > items;
	int_queue->Remove_Items( items );

	ASSERT_TRUE( items[ 0 ] == 5 );
	ASSERT_TRUE( items[ 1 ] == 10 );
	ASSERT_TRUE( items[ 2 ] == 15 );
}

static const IP::String LOG_MESSAGE_1( "Log Message 1" );
static const IP::String LOG_MESSAGE_2( "Log Message 2" );

TEST( ConcurrentQueueTests, Add_Remove_Shared_Ptr_Locking )
{
	auto message_queue = IP::Make_Unique< CLockingConcurrentQueue< IP::UniquePtr< const CLogRequestMessage > > >( MEMORY_TAG );

	IP::UniquePtr< const CLogRequestMessage > message1( IP::New< CLogRequestMessage >( MEMORY_TAG, MANAGER_PROCESS_PROPERTIES, LOG_MESSAGE_1 ) );
	message_queue->Move_Item( std::move( message1 ) );

	IP::UniquePtr< const CLogRequestMessage > message2( IP::New< CLogRequestMessage >( MEMORY_TAG, MANAGER_PROCESS_PROPERTIES, LOG_MESSAGE_2 ) );
	message_queue->Move_Item( std::move( message2 ) );

	IP::Vector< IP::UniquePtr< const CLogRequestMessage > > items;
	message_queue->Remove_Items( items );

	ASSERT_TRUE( items[ 0 ]->Get_Message() == LOG_MESSAGE_1 );
	ASSERT_TRUE( items[ 0 ]->Get_Source_Properties() == MANAGER_PROCESS_PROPERTIES );
	ASSERT_TRUE( items[ 1 ]->Get_Message() == LOG_MESSAGE_2 );
	ASSERT_TRUE( items[ 1 ]->Get_Source_Properties() == MANAGER_PROCESS_PROPERTIES );
}

TEST( ConcurrentQueueTests, Add_Remove_Shared_Ptr_Lockless )
{
	auto message_queue = IP::Make_Unique< CTBBConcurrentQueue< IP::UniquePtr< const CLogRequestMessage > > >( MEMORY_TAG );

	IP::UniquePtr< const CLogRequestMessage > message1( IP::New< CLogRequestMessage >( MEMORY_TAG, MANAGER_PROCESS_PROPERTIES, LOG_MESSAGE_1 ) );
	message_queue->Move_Item( std::move( message1 ) );

	IP::UniquePtr< const CLogRequestMessage > message2( IP::New< CLogRequestMessage >( MEMORY_TAG, MANAGER_PROCESS_PROPERTIES, LOG_MESSAGE_2 ) );
	message_queue->Move_Item( std::move( message2 ) );

	IP::Vector< IP::UniquePtr< const CLogRequestMessage > > items;
	message_queue->Remove_Items( items );

	ASSERT_TRUE( items[ 0 ]->Get_Message() == LOG_MESSAGE_1 );
	ASSERT_TRUE( items[ 0 ]->Get_Source_Properties() == MANAGER_PROCESS_PROPERTIES );
	ASSERT_TRUE( items[ 1 ]->Get_Message() == LOG_MESSAGE_2 );
	ASSERT_TRUE( items[ 1 ]->Get_Source_Properties() == MANAGER_PROCESS_PROPERTIES );
}