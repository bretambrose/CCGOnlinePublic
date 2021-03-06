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

#include "IPShared/Concurrency/ProcessMailbox.h"
#include "IPShared/Concurrency/MailboxInterfaces.h"
#include "IPShared/Concurrency/ProcessMessageFrame.h"
#include "IPShared/Concurrency/Messaging/LoggingMessages.h"
#include "IPShared/Concurrency/ProcessConstants.h"
#include "IPShared/Concurrency/ProcessID.h"

using namespace IP::Execution;
using namespace IP::Execution::Messaging;

static const std::wstring LOG_MESSAGES[] = {
	std::wstring( L"Message 1-1" ),
	std::wstring( L"Message 1-2" ),
	std::wstring( L"Message 2-1" ),
	std::wstring( L"Message 2-2" )
};

TEST( VirtualProcessMailboxTests, Add_Remove )
{
	std::unique_ptr< CProcessMailbox > mailbox( new CProcessMailbox( EProcessID::LOGGING, LOGGING_PROCESS_PROPERTIES ) );

	std::unique_ptr< CProcessMessageFrame > frame1( new CProcessMessageFrame( EProcessID::CONCURRENCY_MANAGER ) );
	frame1->Add_Message( std::unique_ptr< const IProcessMessage >( new CLogRequestMessage( MANAGER_PROCESS_PROPERTIES, LOG_MESSAGES[ 0 ] ) ) );
	frame1->Add_Message( std::unique_ptr< const IProcessMessage >( new CLogRequestMessage( MANAGER_PROCESS_PROPERTIES, LOG_MESSAGES[ 1 ] ) ) );

	std::unique_ptr< CProcessMessageFrame > frame2( new CProcessMessageFrame( EProcessID::CONCURRENCY_MANAGER ) );
	frame2->Add_Message( std::unique_ptr< const IProcessMessage >( new CLogRequestMessage( MANAGER_PROCESS_PROPERTIES, LOG_MESSAGES[ 2 ] ) ) );
	frame2->Add_Message( std::unique_ptr< const IProcessMessage >( new CLogRequestMessage( MANAGER_PROCESS_PROPERTIES, LOG_MESSAGES[ 3 ] ) ) );

	std::shared_ptr< CWriteOnlyMailbox > write_interface = mailbox->Get_Writable_Mailbox();
	write_interface->Add_Frame( frame1 );
	write_interface->Add_Frame( frame2 );

	std::vector< std::unique_ptr< CProcessMessageFrame > > frames;
	std::shared_ptr< CReadOnlyMailbox > read_interface = mailbox->Get_Readable_Mailbox();
	read_interface->Remove_Frames( frames );

	uint32_t log_index = 0;
	for ( uint32_t i = 0; i < frames.size(); ++i )
	{
		std::unique_ptr< CProcessMessageFrame > &frame = frames[ i ];

		for ( auto iter = frame->cbegin(), end = frame->cend(); iter != end; ++iter )
		{
			const CLogRequestMessage *log_request = static_cast< const CLogRequestMessage * >( iter->get() );
			ASSERT_TRUE( log_request->Get_Message() == LOG_MESSAGES[ log_index ] );

			++log_index;
		}
	}
}