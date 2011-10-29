/**********************************************************************************************************************

	[Placeholder for eventual source license]

	ThreadConnectionTests.cpp
		defines unit tests for thread connection related functionality

	(c) Copyright 2011, Bret Ambrose.  All rights reserved.

**********************************************************************************************************************/

#include "stdafx.h"

#include "Concurrency/ThreadConnection.h"
#include "Concurrency/ThreadInterfaces.h"
#include "Concurrency/ThreadMessageFrame.h"
#include "Concurrency/ThreadMessages/LoggingMessages.h"
#include "Concurrency/ThreadConstants.h"

static const std::wstring LOG_MESSAGES[] = {
	std::wstring( L"Message 1-1" ),
	std::wstring( L"Message 1-2" ),
	std::wstring( L"Message 2-1" ),
	std::wstring( L"Message 2-2" )
};

TEST( ThreadConnectionTests, Add_Remove )
{
	CThreadConnection *connection = new CThreadConnection( LOG_THREAD_KEY );

	shared_ptr< CThreadMessageFrame > frame1( new CThreadMessageFrame( MANAGER_THREAD_KEY ) );
	frame1->Add_Message( shared_ptr< const IThreadMessage >( new CLogRequestMessage( MANAGER_THREAD_KEY, LOG_MESSAGES[ 0 ] ) ) );
	frame1->Add_Message( shared_ptr< const IThreadMessage >( new CLogRequestMessage( MANAGER_THREAD_KEY, LOG_MESSAGES[ 1 ] ) ) );

	shared_ptr< CThreadMessageFrame > frame2( new CThreadMessageFrame( MANAGER_THREAD_KEY ) );
	frame2->Add_Message( shared_ptr< const IThreadMessage >( new CLogRequestMessage( MANAGER_THREAD_KEY, LOG_MESSAGES[ 2 ] ) ) );
	frame2->Add_Message( shared_ptr< const IThreadMessage >( new CLogRequestMessage( MANAGER_THREAD_KEY, LOG_MESSAGES[ 3 ] ) ) );

	shared_ptr< CWriteOnlyThreadInterface > write_interface = connection->Get_Writer_Interface();
	write_interface->Add_Frame( frame1 );
	write_interface->Add_Frame( frame2 );

	std::vector< shared_ptr< CThreadMessageFrame > > frames;
	shared_ptr< CReadOnlyThreadInterface > read_interface = connection->Get_Reader_Interface();
	read_interface->Remove_Frames( frames );

	uint32 log_index = 0;
	for ( uint32 i = 0; i < frames.size(); ++i )
	{
		shared_ptr< CThreadMessageFrame > frame = frames[ i ];
		for ( auto iter = frame->Get_Frame_Begin(); iter != frame->Get_Frame_End(); ++iter )
		{
			shared_ptr< const CLogRequestMessage > log_request = static_pointer_cast< const CLogRequestMessage >( *iter );
			ASSERT_TRUE( log_request->Get_Message() == LOG_MESSAGES[ log_index ] );

			++log_index;
		}
	}

	delete connection;
}