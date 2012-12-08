/**********************************************************************************************************************

	ProcessBase.cpp
		A component containing the logic shared by all processes.

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

#include "MessageHandling/ProcessMessageHandler.h"
#include "TaskScheduler/TaskScheduler.h"
#include "ProcessSubject.h"
#include "MailboxInterfaces.h"
#include "ProcessConstants.h"
#include "ProcessExecutionMode.h"
#include "ProcessMessageFrame.h"
#include "Messaging/LoggingMessages.h"
#include "Messaging/ProcessManagementMessages.h"
#include "Messaging/ExchangeMailboxMessages.h"
#include "ProcessID.h"

enum EProcessState
{
	EPS_INITIALIZING,
	EPS_RUNNING,
	EPS_SHUTTING_DOWN_SOFT,
	EPS_SHUTTING_DOWN_HARD
};

/**********************************************************************************************************************
	CProcessBase::CProcessBase -- constructor
	
		properties -- the properties of this process
				
**********************************************************************************************************************/
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

/**********************************************************************************************************************
	CProcessBase::~CProcessBase -- destructor
					
**********************************************************************************************************************/
CProcessBase::~CProcessBase()
{
	Cleanup();
}

/**********************************************************************************************************************
	CProcessBase::Initialize -- initializes the virtual process

		id -- assigned id of this process
					
**********************************************************************************************************************/
void CProcessBase::Initialize( EProcessID::Enum id )
{
	ID = id;
	Register_Message_Handlers();
}

/**********************************************************************************************************************
	CProcessBase::Cleanup -- cleans up the process
					
**********************************************************************************************************************/
void CProcessBase::Cleanup( void )
{
	MessageHandlers.clear();	// not really necessary
}

/**********************************************************************************************************************
	CProcessBase::Log -- requests a message be written to the log file for this virtual process

		message -- message to output to the log file
					
**********************************************************************************************************************/
void CProcessBase::Log( const std::wstring &message )
{
	// actual logging thread should override this function and never call the baseclass
	FATAL_ASSERT( ID != EProcessID::LOGGING );

	Send_Process_Message( EProcessID::LOGGING, shared_ptr< const IProcessMessage >( new CLogRequestMessage( Properties, message ) ) );
}

/**********************************************************************************************************************
	CProcessBase::Get_Mailbox -- gets the mailbox for a process, if we have it

		process_id -- id of the process to get a mailbox for
					
**********************************************************************************************************************/
shared_ptr< CWriteOnlyMailbox > CProcessBase::Get_Mailbox( EProcessID::Enum process_id ) const
{
	auto interface_iter = Mailboxes.find( process_id );
	if ( interface_iter != Mailboxes.end() )
	{
		return interface_iter->second;
	}

	return shared_ptr< CWriteOnlyMailbox >( nullptr );
}

/**********************************************************************************************************************
	CProcessBase::Send_Process_Message -- sends a message to another process

		dest_process_id -- id of the process to send to
		message -- message to send
					
**********************************************************************************************************************/
void CProcessBase::Send_Process_Message( EProcessID::Enum dest_process_id, const shared_ptr< const IProcessMessage > &message )
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
		if ( iter == PendingOutboundFrames.end() )
		{
			shared_ptr< CProcessMessageFrame > frame( new CProcessMessageFrame( ID ) );
			frame->Add_Message( message );
			PendingOutboundFrames.insert( FrameTableType::value_type( dest_process_id, frame ) );
			return;
		}

		iter->second->Add_Message( message );
	}
}

/**********************************************************************************************************************
	CProcessBase::Send_Manager_Message -- sends a message to the concurrency manager

		message -- message to send
					
**********************************************************************************************************************/
void CProcessBase::Send_Manager_Message( const shared_ptr< const IProcessMessage > &message )
{
	Send_Process_Message( EProcessID::CONCURRENCY_MANAGER, message );
}

/**********************************************************************************************************************
	CProcessBase::Flush_Regular_Messages -- sends all pending messages to their destination process; does not include
		manager or logging processes
					
**********************************************************************************************************************/
void CProcessBase::Flush_Regular_Messages( void )
{
	if ( !Is_Shutting_Down() )
	{
		// under normal circumstances, send all messages to threads we have an interface for
		std::vector< EProcessID::Enum > sent_frames;

		for ( auto frame_iterator = PendingOutboundFrames.cbegin(); frame_iterator != PendingOutboundFrames.cend(); ++frame_iterator )
		{
			shared_ptr< CWriteOnlyMailbox > writeable_mailbox = Get_Mailbox( frame_iterator->first );
			if ( writeable_mailbox != nullptr )
			{
				writeable_mailbox->Add_Frame( frame_iterator->second );
				sent_frames.push_back( frame_iterator->first );
			}
		}

		// erase only the frames that we sent
		for ( uint32 i = 0; i < sent_frames.size(); i++ )
		{
			PendingOutboundFrames.erase( sent_frames[ i ] );
		}
	}
	else
	{
		// when shutting down, we sometimes restrict what threads can be sent to depending on the nature of the shut down
		// soft shut downs allow messages to be sent arbitrarily, hard shutdowns restrict to manager or log thread only
		for ( auto frame_iterator = PendingOutboundFrames.cbegin(); frame_iterator != PendingOutboundFrames.cend(); ++frame_iterator )
		{
			if ( State == EPS_SHUTTING_DOWN_SOFT || frame_iterator->first == EProcessID::CONCURRENCY_MANAGER || frame_iterator->first == EProcessID::LOGGING )
			{
				shared_ptr< CWriteOnlyMailbox > writeable_mailbox = Get_Mailbox( frame_iterator->first );
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

/**********************************************************************************************************************
	CProcessBase::Set_Manager_Mailbox -- sets the write-only interface to the concurrency manager

		mailbox -- the manager's write interface
					
**********************************************************************************************************************/
void CProcessBase::Set_Manager_Mailbox( const shared_ptr< CWriteOnlyMailbox > &mailbox )
{
	FATAL_ASSERT( ManagerMailbox.get() == nullptr );

	ManagerMailbox = mailbox;
}

/**********************************************************************************************************************
	CProcessBase::Set_Logging_Mailbox -- sets the write-only interface to the logging process

		mailbox -- the logging process's write interface
					
**********************************************************************************************************************/
void CProcessBase::Set_Logging_Mailbox( const shared_ptr< CWriteOnlyMailbox > &mailbox )
{
	FATAL_ASSERT( LoggingMailbox.get() == nullptr );

	LoggingMailbox = mailbox;
}

/**********************************************************************************************************************
	CProcessBase::Set_My_Mailbox -- sets the read-only mailbox you own

		mailbox -- our read-only mailbox
					
**********************************************************************************************************************/
void CProcessBase::Set_My_Mailbox( const shared_ptr< CReadOnlyMailbox > &mailbox )
{
	MyMailbox = mailbox;
}

/**********************************************************************************************************************
	CProcessBase::Get_Next_Task_Time -- gets the next time in the future when a scheduled task should execute

		Returns: next scheduled task time
					
**********************************************************************************************************************/
double CProcessBase::Get_Next_Task_Time( void ) const
{
	return TaskScheduler->Get_Next_Task_Time();
}

/**********************************************************************************************************************
	CProcessBase::Run -- base execution logic

		context -- the tbb context that this process is being run under
					
**********************************************************************************************************************/
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

/**********************************************************************************************************************
	CProcessBase::Service_Message_Frames -- handles all incoming process messages
					
**********************************************************************************************************************/
void CProcessBase::Service_Message_Frames( void )
{
	if ( MyMailbox.get() == nullptr )
	{
		return;
	}

	// get all the queued incoming messages
	std::vector< shared_ptr< CProcessMessageFrame > > frames;
	MyMailbox->Remove_Frames( frames );

	// iterate each frame
	for ( uint32 i = 0; i < frames.size(); ++i )
	{
		const shared_ptr< CProcessMessageFrame > &frame = frames[ i ];
		EProcessID::Enum source_process_id = frame->Get_Process_ID();

		// iterate each message within the frame
		for ( auto iter = frame->Get_Frame_Begin(); iter != frame->Get_Frame_End(); ++iter )
		{
			Handle_Message( source_process_id, *iter );
		}
	}
}

/**********************************************************************************************************************
	CProcessBase::Handle_Shutdown_Mailboxes -- services all mailboxes that are pending shutdown, either erasing
		an associated outbound frame if we don't have the mailbox, or erasing the mailbox.  Notifies the manager
		that each mailbox has been released.
					
**********************************************************************************************************************/
void CProcessBase::Handle_Shutdown_Mailboxes( void )
{
	for ( auto iter = ShutdownMailboxes.cbegin(); iter != ShutdownMailboxes.cend(); ++iter )
	{
		EProcessID::Enum process_id = *iter;

		auto interface_iter = Mailboxes.find( process_id );
		if ( interface_iter == Mailboxes.end() )
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
			FATAL_ASSERT( PendingOutboundFrames.find( process_id ) == PendingOutboundFrames.end() );

			Mailboxes.erase( interface_iter );
		}

		Remove_Process_ID_From_Tables( process_id );

		// let the manager know we've release this interface
		Send_Manager_Message( shared_ptr< const IProcessMessage >( new CReleaseMailboxResponse( process_id ) ) );
	}

	ShutdownMailboxes.clear();
}

/**********************************************************************************************************************
	CProcessBase::Should_Reschedule -- should this task be rescheduled

		Returns: true if it should be rescheduled, otherwise false
					
**********************************************************************************************************************/
bool CProcessBase::Should_Reschedule( void ) const
{
	return State == EPS_RUNNING && Get_Execution_Mode() == EProcessExecutionMode::TASK;
}

/**********************************************************************************************************************
	CProcessBase::Handle_Message -- central message handling dispatcher

		key -- message sender
		message -- the process message to handle
					
**********************************************************************************************************************/
void CProcessBase::Handle_Message( EProcessID::Enum process_id, const shared_ptr< const IProcessMessage > &message )
{
	const IProcessMessage *msg_base = message.get();

	Loki::TypeInfo hash_key( typeid( *msg_base ) );
	auto iter = MessageHandlers.find( hash_key );
	FATAL_ASSERT( iter != MessageHandlers.end() );

	iter->second->Handle_Message( process_id, message );
}

/**********************************************************************************************************************
	CProcessBase::Register_Message_Handlers -- creates message handlers for each message that we want to receive
					
**********************************************************************************************************************/
void CProcessBase::Register_Message_Handlers( void )
{
	REGISTER_THIS_HANDLER( CAddMailboxMessage, CProcessBase, Handle_Add_Mailbox_Message )
	REGISTER_THIS_HANDLER( CReleaseMailboxRequest, CProcessBase, Handle_Release_Mailbox_Request )
	REGISTER_THIS_HANDLER( CShutdownSelfRequest, CProcessBase, Handle_Shutdown_Self_Request )
} 

/**********************************************************************************************************************
	CProcessBase::Register_Handler -- registers a message handlers for a process message

		message_type_info -- the C++ type of the message class
		handler -- message handling delegate
					
**********************************************************************************************************************/
void CProcessBase::Register_Handler( const std::type_info &message_type_info, const shared_ptr< IProcessMessageHandler > &handler )
{
	Loki::TypeInfo key( message_type_info );

	FATAL_ASSERT( MessageHandlers.find( key ) == MessageHandlers.end() );

	MessageHandlers[ key ] = handler;
}

/**********************************************************************************************************************
	CProcessBase::Handle_Add_Mailbox_Message -- handles the AddMailboxMessage message

		source_process_id -- id of the process source of the message
		message -- the AddMailboxMessage message
					
**********************************************************************************************************************/
void CProcessBase::Handle_Add_Mailbox_Message( EProcessID::Enum /*source_process_id*/, const shared_ptr< const CAddMailboxMessage > &message )
{
	EProcessID::Enum add_id = message->Get_Mailbox()->Get_Process_ID();
	FATAL_ASSERT( add_id != EProcessID::CONCURRENCY_MANAGER && add_id != EProcessID::LOGGING );

	if ( Mailboxes.find( add_id ) == Mailboxes.end() )
	{
		Mailboxes.insert( MailboxTableType::value_type( add_id, message->Get_Mailbox() ) );

		const SProcessProperties &properties = message->Get_Mailbox()->Get_Properties();
		IDToPropertiesTable.insert( IDToProcessPropertiesTableType::value_type( add_id, properties ) );
		PropertiesToIDTable.insert( ProcessPropertiesToIDTableType::value_type( properties, add_id ) );
	}
}

/**********************************************************************************************************************
	CProcessBase::Handle_Release_Mailbox_Request -- handles the ReleaseMailboxRequest message

		source_process_id -- id of the process source of the message
		message -- message to handle
					
**********************************************************************************************************************/
void CProcessBase::Handle_Release_Mailbox_Request( EProcessID::Enum source_process_id, const shared_ptr< const CReleaseMailboxRequest > &request )
{
	FATAL_ASSERT( source_process_id == EProcessID::CONCURRENCY_MANAGER );

	EProcessID::Enum shutdown_process_id = request->Get_Process_ID();
	FATAL_ASSERT( shutdown_process_id != EProcessID::CONCURRENCY_MANAGER && shutdown_process_id != EProcessID::LOGGING );

	ShutdownMailboxes.insert( shutdown_process_id );
}

/**********************************************************************************************************************
	CProcessBase::Handle_Shutdown_Self_Request -- handles the ShutdownSelf request

		source_process_id -- id of the process source of the message
		message -- the ShutdownThread request
					
**********************************************************************************************************************/
void CProcessBase::Handle_Shutdown_Self_Request( EProcessID::Enum source_process_id, const shared_ptr< const CShutdownSelfRequest > &message )
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

	Send_Manager_Message( shared_ptr< const IProcessMessage >( new CShutdownSelfResponse() ) );	
}

/**********************************************************************************************************************
	CProcessBase::Is_Shutting_Down -- is this process in the process of shutting down?

		Returns: true if shutting down, false otherwise
					
**********************************************************************************************************************/
bool CProcessBase::Is_Shutting_Down( void ) const
{
	return State == EPS_SHUTTING_DOWN_SOFT || State == EPS_SHUTTING_DOWN_HARD;
}

/**********************************************************************************************************************
	CProcessBase::Flush_System_Messages -- Manager and log messages get sent separately from other messages.
		This function must be the last function called in this thread's execution context/tbb-execute.  The instant a reschedule
		message is pushed to the manager, this thread may end up getting reexecuted which would cause data corruption
		if the current execution is still ongoing.
					
**********************************************************************************************************************/
void CProcessBase::Flush_System_Messages( void )
{
	bool is_shutting_down = Is_Shutting_Down();

	// Flush logging messages if possible
	if ( LoggingMailbox.get() != nullptr && LogFrame.get() != nullptr )
	{
		shared_ptr< CProcessMessageFrame > log_frame( LogFrame );
		LogFrame.reset();

		LoggingMailbox->Add_Frame( log_frame );
	}

	// Clear log interface if necessary
	if ( is_shutting_down )
	{
		LoggingMailbox.reset();
	}

	// Flush manager messages if possible
	if ( ManagerMailbox.get() != nullptr && ManagerFrame.get() != nullptr )
	{
		shared_ptr< CProcessMessageFrame > manager_frame( ManagerFrame );
		ManagerFrame.reset();

		ManagerMailbox->Add_Frame( manager_frame );
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

/**********************************************************************************************************************
	CProcessBase::Remove_Process_ID_From_Tables -- removes a process id from the pair of tables mapping
		process properties to/from process ids

		process_id -- id of the process to remove all references to
					
**********************************************************************************************************************/
void CProcessBase::Remove_Process_ID_From_Tables( EProcessID::Enum process_id )
{
	IDToProcessPropertiesTableType::iterator iter1 = IDToPropertiesTable.find( process_id );
	if ( iter1 == IDToPropertiesTable.end() )
	{
		return;
	}

	ProcessPropertiesToIDTableType::iterator ub_iter2 = PropertiesToIDTable.upper_bound( iter1->second );
	for ( ProcessPropertiesToIDTableType::iterator iter2 = PropertiesToIDTable.lower_bound( iter1->second ); iter2 != ub_iter2; ++iter2 )
	{
		if ( iter2->second == process_id )
		{
			PropertiesToIDTable.erase( iter2 );
			break;
		}
	}

	IDToPropertiesTable.erase( iter1 );
}

/**********************************************************************************************************************
	CProcessBase::Build_Process_ID_List_By_Properties -- builds a list of all known processes that match
		a property set

		properties -- properties to match against
		process_ids -- output vector of matching process ids
					
**********************************************************************************************************************/
void CProcessBase::Build_Process_ID_List_By_Properties( const SProcessProperties &properties, std::vector< EProcessID::Enum > &process_ids ) const
{
	process_ids.clear();

	for ( IDToProcessPropertiesTableType::const_iterator iter = IDToPropertiesTable.cbegin(); iter != IDToPropertiesTable.cend(); ++iter )
	{
		if ( properties.Matches( iter->second ) )
		{
			process_ids.push_back( iter->first );
		}
	}
}

