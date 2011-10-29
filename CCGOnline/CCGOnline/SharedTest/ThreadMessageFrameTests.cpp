/**********************************************************************************************************************

	[Placeholder for eventual source license]

	ThreadMessageFrameTests.cpp
		defines unit tests for thread message frame related functionality

	(c) Copyright 2011, Bret Ambrose.  All rights reserved.

**********************************************************************************************************************/

#include "stdafx.h"

#include "Concurrency/ThreadMessages/LoggingMessages.h"
#include "Concurrency/ThreadConstants.h"
#include "Concurrency/ThreadMessageFrame.h"

static const std::wstring LOG_MESSAGES[] = {
	std::wstring( L"Help I'm a message" ),
	std::wstring( L"Blah" )
};

TEST( ThreadMessageFrameTests, Add_Remove )
{
	CThreadMessageFrame message_frame( LOG_THREAD_KEY );

	message_frame.Add_Message( shared_ptr< const IThreadMessage >( new CLogRequestMessage( MANAGER_THREAD_KEY, LOG_MESSAGES[ 0 ] ) ) );
	message_frame.Add_Message( shared_ptr< const IThreadMessage >( new CLogRequestMessage( MANAGER_THREAD_KEY, LOG_MESSAGES[ 1 ] ) ) );

	uint32 i = 0;
	for ( auto iter = message_frame.Get_Frame_Begin(); iter != message_frame.Get_Frame_End(); ++iter, ++i )
	{
		shared_ptr< const CLogRequestMessage > base_message = static_pointer_cast< const CLogRequestMessage >( *iter );
		ASSERT_TRUE( base_message->Get_Message() == LOG_MESSAGES[ i ] );
	}
}