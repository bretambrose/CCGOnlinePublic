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

#include "ProcessBase.h"


#include "IPShared/MessageHandling/ProcessMessageHandler.h"
#include "IPShared/TaskScheduler/TaskScheduler.h"
#include "ProcessSubject.h"
#include "MailboxInterfaces.h"
#include "ProcessConstants.h"
#include "ProcessMessageFrame.h"
#include "Messaging/LoggingMessages.h"
#include "Messaging/ProcessManagementMessages.h"
#include "Messaging/ExchangeMailboxMessages.h"
#include "ProcessID.h"

namespace IP
{
namespace Execution
{

enum EProcessState
{
	EPS_INITIALIZING,
	EPS_RUNNING,
	EPS_SHUTTING_DOWN_SOFT,
	EPS_SHUTTING_DOWN_HARD
};


CProcessBase::CProcessBase( const SProcessProperties &properties ) :
	BASECLASS(),
	ID( EProcessID::INVALID ),
	Properties( properties ),
	State( EPS_INITIALIZING ),
	PendingOutboundFrames(),
	ManagerFrame( nullptr ),
	LogFrame( nullptr ),
	IDToPropertiesTable(),
	PropertiesToIDTable(),
	Mailboxes(),
	ManagerMailbox( nullptr ),
	LoggingMailbox( nullptr ),
	MyMailbox( nullptr ),
	ShutdownMailboxes(),
	FirstServiceTimeSeconds( 0.0 ),
	CurrentTimeSeconds( 0.0 ),
	MessageHandlers(),
	TaskScheduler( new CTaskScheduler )
{
}


CProcessBase::~CProcessBase()
{
	Cleanup();
}


void CProcessBase::Initialize( EProcessID id )
{
	ID = id;
	Register_Message_Handlers();
}


void CProcessBase::Cleanup( void )
{
	MessageHandlers.clear();	// not really necessary
}


void CProcessBase::Log( std::wstring &&message )
{
	// actual logging thread should override this function and never call the baseclass
	FATAL_ASSERT( ID != EProcessID::LOGGING );

	std::unique_ptr< const Messaging::IProcessMessage > log_request( new Messaging::CLogRequestMessage( Properties, std::move( message ) ) );
	Send_Process_Message( EProcessID::LOGGING, log_request );
}


std::shared_ptr< CWriteOnlyMailbox > CProcessBase::Get_Mailbox( EProcessID process_id ) const
{
	auto interface_iter = Mailboxes.find( process_id );
	if ( interface_iter != Mailboxes.cend() )
	{
		return interface_iter->second;
	}

	return std::shared_ptr< CWriteOnlyMailbox >( nullptr );
}


void CProcessBase::Send_Process_Message( EProcessID dest_process_id, std::unique_ptr< const Messaging::IProcessMessage > &message )
{
	Send_Process_Message( dest_process_id, std::move( message ) );
}


void CProcessBase::Send_Process_Message( EProcessID dest_process_id, std::unique_ptr< const Messaging::IProcessMessage > &&message )
{
	// manager and logging threads are special-cased in order to avoid race conditions related to rescheduling
	if ( dest_process_id == EProcessID::CONCURRENCY_MANAGER )
	{
		if ( ManagerFrame.get() == nullptr )
		{
			ManagerFrame.reset( new CProcessMessageFrame( ID ) );
		}
		
		ManagerFrame->Add_Message( message );
	}
	else if ( dest_process_id == EProcessID::LOGGING )
	{
		if ( LogFrame.get() == nullptr )
		{
			LogFrame.reset( new CProcessMessageFrame( ID ) );
		}
		
		LogFrame->Add_Message( message );
	}
	else
	{
		// find or create a frame for the destination thread
		auto iter = PendingOutboundFrames.find( dest_process_id );
		if ( iter == PendingOutboundFrames.cend() )
		{
			std::unique_ptr< CProcessMessageFrame > frame( new CProcessMessageFrame( ID ) );
			frame->Add_Message( message );
			PendingOutboundFrames.insert( FrameTableType::value_type( dest_process_id, std::move( frame ) ) );
			return;
		}

		iter->second->Add_Message( message );
	}
}


void CProcessBase::Send_Manager_Message( std::unique_ptr< const Messaging::IProcessMessage > &message )
{
	Send_Process_Message( EProcessID::CONCURRENCY_MANAGER, message );
}


void CProcessBase::Send_Manager_Message( std::unique_ptr< const Messaging::IProcessMessage > &&message )
{
	Send_Process_Message( EProcessID::CONCURRENCY_MANAGER, message );
}


void CProcessBase::Flush_Regular_Messages( void )
{
	if ( !Is_Shutting_Down() )
	{
		// under normal circumstances, send all messages to threads we have an interface for
		std::vector< EProcessID > sent_frames;

		for ( auto frame_iterator = PendingOutboundFrames.begin(), end = PendingOutboundFrames.end(); frame_iterator != end; ++frame_iterator )
		{
			std::shared_ptr< CWriteOnlyMailbox > writeable_mailbox = Get_Mailbox( frame_iterator->first );
			if ( writeable_mailbox != nullptr )
			{
				writeable_mailbox->Add_Frame( frame_iterator->second );
				sent_frames.push_back( frame_iterator->first );
			}
		}

		// erase only the frames that we sent
		for ( size_t i = 0, size = sent_frames.size(); i < size; i++ )
		{
			PendingOutboundFrames.erase( sent_frames[ i ] );
		}
	}
	else
	{
		// when shutting down, we sometimes restrict what threads can be sent to depending on the nature of the shut down
		// soft shut downs allow messages to be sent arbitrarily, hard shutdowns restrict to manager or log thread only
		for ( auto frame_iterator = PendingOutboundFrames.begin(), end = PendingOutboundFrames.end(); frame_iterator != end; ++frame_iterator )
		{
			if ( State == EPS_SHUTTING_DOWN_SOFT || frame_iterator->first == EProcessID::CONCURRENCY_MANAGER || frame_iterator->first == EProcessID::LOGGING )
			{
				std::shared_ptr< CWriteOnlyMailbox > writeable_mailbox = Get_Mailbox( frame_iterator->first );
				if ( writeable_mailbox != nullptr )
				{
					writeable_mailbox->Add_Frame( frame_iterator->second );
				}
			}
		}

		// unlike normal operations, when shutting down, we remove all pending messages whether or not they were sent
		PendingOutboundFrames.clear();
	}
}


void CProcessBase::Set_Manager_Mailbox( const std::shared_ptr< CWriteOnlyMailbox > &mailbox )
{
	FATAL_ASSERT( ManagerMailbox.get() == nullptr );

	ManagerMailbox = mailbox;
}


void CProcessBase::Set_Logging_Mailbox( const std::shared_ptr< CWriteOnlyMailbox > &mailbox )
{
	FATAL_ASSERT( LoggingMailbox.get() == nullptr );

	LoggingMailbox = mailbox;
}


void CProcessBase::Set_My_Mailbox( const std::shared_ptr< CReadOnlyMailbox > &mailbox )
{
	MyMailbox = mailbox;
}


double CProcessBase::Get_Next_Task_Time( void ) const
{
	return TaskScheduler->Get_Next_Task_Time();
}


void CProcessBase::Run( const CProcessExecutionContext & /*context*/ )
{
	if ( State == EPS_INITIALIZING )
	{
		State = EPS_RUNNING;
	}

	if ( Is_Shutting_Down() )
	{
		return;
	}

	Per_Frame_Logic_Start();

	// message and scheduled task logic
	Service_Message_Frames();

	TaskScheduler->Service( Get_Current_Process_Time() );

	Per_Frame_Logic_End();

	FATAL_ASSERT( !( Should_Reschedule() && Is_Shutting_Down() ) );

	Service_Reschedule();

	// Awkward but necessary due to shutdown sequence
	// We need to flush before we call Handle_Shutdown_Mailboxes, but that function also needs to send messages that go out this moment too,
	// so we do before-and-after flushes
	Flush_Regular_Messages();
	Handle_Shutdown_Mailboxes();
	Flush_Regular_Messages();

	if ( Is_Shutting_Down() )
	{
		FATAL_ASSERT( PendingOutboundFrames.size() == 0 );
		Mailboxes.clear();
	}
}


void CProcessBase::Service_Message_Frames( void )
{
	if ( MyMailbox.get() == nullptr )
	{
		return;
	}

	// get all the queued incoming messages
	std::vector< std::unique_ptr< CProcessMessageFrame > > frames;
	MyMailbox->Remove_Frames( frames );

	// iterate each frame
	for ( uint32_t i = 0; i < frames.size(); ++i )
	{
		std::unique_ptr< CProcessMessageFrame > &frame = frames[ i ];
		EProcessID source_process_id = frame->Get_Process_ID();

		// iterate all messages in the frame
		for ( auto iter = frame->begin(), end = frame->end(); iter != end; ++iter )
		{
			Handle_Message( source_process_id, *iter );
		}
	}
}


void CProcessBase::Handle_Shutdown_Mailboxes( void )
{
	for ( auto iter = ShutdownMailboxes.cbegin(), end = ShutdownMailboxes.cend(); iter != end; ++iter )
	{
		EProcessID process_id = *iter;

		auto interface_iter = Mailboxes.find( process_id );
		if ( interface_iter == Mailboxes.cend() )
		{
			auto frame_iter = PendingOutboundFrames.find( process_id );
			if ( frame_iter != PendingOutboundFrames.end() )
			{
				PendingOutboundFrames.erase( frame_iter );
			}
		}
		else
		{
			// should have been sent in the flush that preceding this call
			FATAL_ASSERT( PendingOutboundFrames.find( process_id ) == PendingOutboundFrames.cend() );

			Mailboxes.erase( interface_iter );
		}

		Remove_Process_ID_From_Tables( process_id );

		// let the manager know we've release this interface
		std::unique_ptr< const Messaging::IProcessMessage > release_msg( new Messaging::CReleaseMailboxResponse( process_id ) );
		Send_Manager_Message( release_msg );
	}

	ShutdownMailboxes.clear();
}


bool CProcessBase::Should_Reschedule( void ) const
{
	return State == EPS_RUNNING && Get_Execution_Mode() == EProcessExecutionMode::TBB_TASK;
}


void CProcessBase::Handle_Message( EProcessID process_id, std::unique_ptr< const Messaging::IProcessMessage > &message )
{
	const Messaging::IProcessMessage *msg_base = message.get();

	Loki::TypeInfo hash_key( typeid( *msg_base ) );
	auto iter = MessageHandlers.find( hash_key );
	FATAL_ASSERT( iter != MessageHandlers.cend() );

	iter->second->Handle_Message( process_id, message );
}


void CProcessBase::Register_Message_Handlers( void )
{
	REGISTER_THIS_HANDLER( Messaging::CAddMailboxMessage, CProcessBase, Handle_Add_Mailbox_Message )
	REGISTER_THIS_HANDLER( Messaging::CReleaseMailboxRequest, CProcessBase, Handle_Release_Mailbox_Request )
	REGISTER_THIS_HANDLER( Messaging::CShutdownSelfRequest, CProcessBase, Handle_Shutdown_Self_Request )
} 


void CProcessBase::Register_Handler( const std::type_info &message_type_info, std::unique_ptr< Messaging::IProcessMessageHandler > &handler )
{
	Loki::TypeInfo key( message_type_info );

	FATAL_ASSERT( MessageHandlers.find( key ) == MessageHandlers.cend() );

	MessageHandlers[ key ] = std::move( handler );
}


void CProcessBase::Handle_Add_Mailbox_Message( EProcessID /*source_process_id*/, std::unique_ptr< const Messaging::CAddMailboxMessage > &message )
{
	EProcessID add_id = message->Get_Mailbox()->Get_Process_ID();
	FATAL_ASSERT( add_id != EProcessID::CONCURRENCY_MANAGER && add_id != EProcessID::LOGGING );

	if ( Mailboxes.find( add_id ) == Mailboxes.cend() )
	{
		Mailboxes.insert( MailboxTableType::value_type( add_id, message->Get_Mailbox() ) );

		const SProcessProperties &properties = message->Get_Mailbox()->Get_Properties();
		IDToPropertiesTable.insert( IDToProcessPropertiesTableType::value_type( add_id, properties ) );
		PropertiesToIDTable.insert( ProcessPropertiesToIDTableType::value_type( properties, add_id ) );
	}
}


void CProcessBase::Handle_Release_Mailbox_Request( EProcessID source_process_id, std::unique_ptr< const Messaging::CReleaseMailboxRequest > &request )
{
	FATAL_ASSERT( source_process_id == EProcessID::CONCURRENCY_MANAGER );

	EProcessID shutdown_process_id = request->Get_Process_ID();
	FATAL_ASSERT( shutdown_process_id != EProcessID::CONCURRENCY_MANAGER && shutdown_process_id != EProcessID::LOGGING );

	ShutdownMailboxes.insert( shutdown_process_id );
}


void CProcessBase::Handle_Shutdown_Self_Request( EProcessID source_process_id, std::unique_ptr< const Messaging::CShutdownSelfRequest > &message )
{
	FATAL_ASSERT( source_process_id == EProcessID::CONCURRENCY_MANAGER );
	FATAL_ASSERT( !Is_Shutting_Down() );

	if ( message->Get_Is_Hard_Shutdown() )
	{
		State = EPS_SHUTTING_DOWN_HARD;
	}
	else
	{
		State = EPS_SHUTTING_DOWN_SOFT;
	}

	On_Shutdown_Self_Request();

	std::unique_ptr< const Messaging::IProcessMessage > shutdown_self_msg( new Messaging::CShutdownSelfResponse() );
	Send_Manager_Message( shutdown_self_msg );	
}


bool CProcessBase::Is_Shutting_Down( void ) const
{
	return State == EPS_SHUTTING_DOWN_SOFT || State == EPS_SHUTTING_DOWN_HARD;
}


void CProcessBase::Flush_System_Messages( void )
{
	bool is_shutting_down = Is_Shutting_Down();

	// Flush logging messages if possible
	if ( LoggingMailbox.get() != nullptr && LogFrame.get() != nullptr )
	{
		LoggingMailbox->Add_Frame( LogFrame );
		LogFrame.reset();
	}

	// Clear log interface if necessary
	if ( is_shutting_down )
	{
		LoggingMailbox.reset();
	}

	// Flush manager messages if possible
	if ( ManagerMailbox.get() != nullptr && ManagerFrame.get() != nullptr )
	{
		ManagerMailbox->Add_Frame( ManagerFrame );
		ManagerFrame.reset();
	}

	// Clear manager interface if necessary
	if ( is_shutting_down )
	{
		// logically safe to do even after we've pushed messages
		// A thread should not be rescheduled if it's in the shut down stage, and even it did
		// get rescheduled, the subsequent execution of the thread does nothing and does not access
		// the manager interface
		ManagerMailbox.reset();	
	}
}


void CProcessBase::Remove_Process_ID_From_Tables( EProcessID process_id )
{
	auto iter1 = IDToPropertiesTable.find( process_id );
	if ( iter1 == IDToPropertiesTable.cend() )
	{
		return;
	}

	auto ub_iter2 = PropertiesToIDTable.upper_bound( iter1->second );
	for ( auto iter2 = PropertiesToIDTable.lower_bound( iter1->second ); iter2 != ub_iter2; ++iter2 )
	{
		if ( iter2->second == process_id )
		{
			PropertiesToIDTable.erase( iter2 );
			break;
		}
	}

	IDToPropertiesTable.erase( iter1 );
}


void CProcessBase::Build_Process_ID_List_By_Properties( const SProcessProperties &properties, std::vector< EProcessID > &process_ids ) const
{
	process_ids.clear();

	for ( auto iter = IDToPropertiesTable.cbegin(), end = IDToPropertiesTable.cend(); iter != end; ++iter )
	{
		if ( properties.Matches( iter->second ) )
		{
			process_ids.push_back( iter->first );
		}
	}
}

} // namespace Execution
} // namespace IP

