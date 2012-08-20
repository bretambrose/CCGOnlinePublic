/**********************************************************************************************************************

	VirtualProcessMessageFrameTests.cpp
		defines unit tests for virtual process message frame related functionality

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

#include "Concurrency/Messaging/LoggingMessages.h"
#include "Concurrency/VirtualProcessConstants.h"
#include "Concurrency/VirtualProcessMessageFrame.h"

static const std::wstring LOG_MESSAGES[] = {
	std::wstring( L"Help I'm a message" ),
	std::wstring( L"Blah" )
};

TEST( VirtualProcessMessageFrameTests, Add_Remove )
{
	CVirtualProcessMessageFrame message_frame( LOG_THREAD_KEY );

	message_frame.Add_Message( shared_ptr< const IVirtualProcessMessage >( new CLogRequestMessage( MANAGER_THREAD_KEY, LOG_MESSAGES[ 0 ] ) ) );
	message_frame.Add_Message( shared_ptr< const IVirtualProcessMessage >( new CLogRequestMessage( MANAGER_THREAD_KEY, LOG_MESSAGES[ 1 ] ) ) );

	uint32 i = 0;
	for ( auto iter = message_frame.Get_Frame_Begin(); iter != message_frame.Get_Frame_End(); ++iter, ++i )
	{
		shared_ptr< const CLogRequestMessage > base_message = static_pointer_cast< const CLogRequestMessage >( *iter );
		ASSERT_TRUE( base_message->Get_Message() == LOG_MESSAGES[ i ] );
	}
}