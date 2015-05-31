/**********************************************************************************************************************

	ProcessMessageFrameTests.cpp
		defines unit tests for process message frame related functionality

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

#include "IPShared/Concurrency/Messaging/LoggingMessages.h"
#include "IPShared/Concurrency/ProcessConstants.h"
#include "IPShared/Concurrency/ProcessMessageFrame.h"
#include "IPShared/Concurrency/ProcessID.h"

static const std::wstring LOG_MESSAGES[] = {
	std::wstring( L"Help I'm a message" ),
	std::wstring( L"Blah" )
};

TEST( VirtualProcessMessageFrameTests, Add_Remove )
{
	CProcessMessageFrame message_frame( EProcessID::LOGGING );

	message_frame.Add_Message( unique_ptr< const IProcessMessage >( new CLogRequestMessage( MANAGER_PROCESS_PROPERTIES, LOG_MESSAGES[ 0 ] ) ) );
	message_frame.Add_Message( unique_ptr< const IProcessMessage >( new CLogRequestMessage( MANAGER_PROCESS_PROPERTIES, LOG_MESSAGES[ 1 ] ) ) );

	uint32_t i = 0;
	for ( auto iter = message_frame.cbegin(), end = message_frame.cend(); iter != end; ++iter, ++i )
	{
		const CLogRequestMessage *log_message = static_cast< const CLogRequestMessage * >( iter->get() );
		ASSERT_TRUE( log_message->Get_Message() == LOG_MESSAGES[ i ] );
	}
}