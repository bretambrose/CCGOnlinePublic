/**********************************************************************************************************************

	VirtualProcessBase.cpp
		A component containing the logic shared by all virtual processes.

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

#include "VirtualProcessBase.h"

#include "MessageHandling/VirtualProcessMessageHandler.h"
#include "TaskScheduler/TaskScheduler.h"
#include "VirtualProcessSubject.h"
#include "MailboxInterfaces.h"
#include "VirtualProcessConstants.h"
#include "VirtualProcessMessageFrame.h"
#include "Messaging/LoggingMessages.h"
#include "Messaging/VirtualProcessManagementMessages.h"
#include "Messaging/ExchangeMailboxMessages.h"
#include "VirtualProcessID.h"

enum EVirtualProcessState
{
	EVPS_INITIALIZING,
	EVPS_RUNNING,
	EVPS_SHUTTING_DOWN_SOFT,
	EVPS_SHUTTING_DOWN_HARD
};

/**********************************************************************************************************************
	CVirtualProcessBase::CVirtualProcessBase -- constructor
	
		properties -- the properties of this virtual process
				
**********************************************************************************************************************/
CVirtualProcessBase::CVirtualProcessBase( const SProcessProperties &properties ) :
	BASECLASS(),
	ID( EVirtualProcessID::INVALID ),
	Properties( properties ),
	State( EVPS_INITIALIZING ),
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
	CVirtualProcessBase::~CVirtualProcessBase -- destructor
					
**********************************************************************************************************************/
CVirtualProcessBase::~CVirtualProcessBase()
{
	Cleanup();
}

/**********************************************************************************************************************
	CVirtualProcessBase::Initialize -- initializes the virtual process

		id -- assigned id of this process
					
**********************************************************************************************************************/
void CVirtualProcessBase::Initialize( EVirtualProcessID::Enum id )
{
	ID = id;
	Register_Message_Handlers();
}

/**********************************************************************************************************************
	CVirtualProcessBase::Cleanup -- cleans up the virtual process
					
**********************************************************************************************************************/
void CVirtualProcessBase::Cleanup( void )
{
	MessageHandlers.clear();	// not really necessary
}

/**********************************************************************************************************************
	CVirtualProcessBase::Log -- requests a message be written to the log file for this virtual process

		message -- message to output to the log file
					
**********************************************************************************************************************/
void CVirtualProcessBase::Log( const std::wstring &message )
{
	// actual logging thread should override this function and never call the baseclass
	FATAL_ASSERT( ID != EVirtualProcessID::LOGGING );

	Send_Virtual_Process_Message( EVirtualProcessID::LOGGING, shared_ptr< const IVirtualProcessMessage >( new CLogRequestMessage( Properties, message ) ) );
}

/**********************************************************************************************************************
	CVirtualProcessBase::Get_Thread_Interface -- gets the mailbox for a thread, if we have it

		process_id -- id of the process to get a mailbox for
					
**********************************************************************************************************************/
shared_ptr< CWriteOnlyMailbox > CVirtualProcessBase::Get_Mailbox( EVirtualProcessID::Enum process_id ) const
{
	auto interface_iter = Mailboxes.find( process_id );
	if ( interface_iter != Mailboxes.end() )
	{
		return interface_iter->second;
	}

	return shared_ptr< CWriteOnlyMailbox >( nullptr );
}

/**********************************************************************************************************************
	CVirtualProcessBase::Send_Virtual_Process_Message -- sends a message to another virtual process

		dest_process_id -- id of the virtual process to send to
		message -- message to send
					
**********************************************************************************************************************/
void CVirtualProcessBase::Send_Virtual_Process_Message( EVirtualProcessID::Enum dest_process_id, const shared_ptr< const IVirtualProcessMessage > &message )
{
	// manager and logging threads are special-cased in order to avoid race conditions related to rescheduling
	if ( dest_process_id == EVirtualProcessID::CONCURRENCY_MANAGER )
	{
		if ( ManagerFrame.get() == nullptr )
		{
			ManagerFrame.reset( new CVirtualProcessMessageFrame( ID ) );
		}
		
		ManagerFrame->Add_Message( message );
	}
	else if ( dest_process_id == EVirtualProcessID::LOGGING )
	{
		if ( LogFrame.get() == nullptr )
		{
			LogFrame.reset( new CVirtualProcessMessageFrame( ID ) );
		}
		
		LogFrame->Add_Message( message );
	}
	else
	{
		// find or create a frame for the destination thread
		auto iter = PendingOutboundFrames.find( dest_process_id );
		if ( iter == PendingOutboundFrames.end() )
		{
			shared_ptr< CVirtualProcessMessageFrame > frame( new CVirtualProcessMessageFrame( ID ) );
			frame->Add_Message( message );
			PendingOutboundFrames.insert( FrameTableType::value_type( dest_process_id, frame ) );
			return;
		}

		iter->second->Add_Message( message );
	}
}

/**********************************************************************************************************************
	CVirtualProcessBase::Flush_Regular_Messages -- sends all pending messages to their destination process; does not include
		manager or logging processes
					
**********************************************************************************************************************/
void CVirtualProcessBase::Flush_Regular_Messages( void )
{
	if ( !Is_Shutting_Down() )
	{
		// under normal circumstances, send all messages to threads we have an interface for
		std::vector< EVirtualProcessID::Enum > sent_frames;

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
			if ( State == EVPS_SHUTTING_DOWN_SOFT || frame_iterator->first == EVirtualProcessID::CONCURRENCY_MANAGER || frame_iterator->first == EVirtualProcessID::LOGGING )
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
	CVirtualProcessBase::Set_Manager_Mailbox -- sets the write-only interface to the concurrency manager

		mailbox -- the manager's write interface
					
**********************************************************************************************************************/
void CVirtualProcessBase::Set_Manager_Mailbox( const shared_ptr< CWriteOnlyMailbox > &mailbox )
{
	FATAL_ASSERT( ManagerMailbox.get() == nullptr );

	ManagerMailbox = mailbox;
}

/**********************************************************************************************************************
	CVirtualProcessBase::Set_Logging_Mailbox -- sets the write-only interface to the logging process

		mailbox -- the logging process's write interface
					
**********************************************************************************************************************/
void CVirtualProcessBase::Set_Logging_Mailbox( const shared_ptr< CWriteOnlyMailbox > &mailbox )
{
	FATAL_ASSERT( LoggingMailbox.get() == nullptr );

	LoggingMailbox = mailbox;
}

/**********************************************************************************************************************
	CVirtualProcessBase::Set_My_Mailbox -- sets the read-only mailbox you own

		mailbox -- our read-only mailbox
					
**********************************************************************************************************************/
void CVirtualProcessBase::Set_My_Mailbox( const shared_ptr< CReadOnlyMailbox > &mailbox )
{
	MyMailbox = mailbox;
}

/**********************************************************************************************************************
	CVirtualProcessBase::Get_Elapsed_Seconds -- gets how many seconds have elapsed since this thread started executing

		Returns: time in seconds that this thread has been executing
					
**********************************************************************************************************************/
double CVirtualProcessBase::Get_Elapsed_Seconds( void ) const
{
	return CurrentTimeSeconds - FirstServiceTimeSeconds;
}

/**********************************************************************************************************************
	CVirtualProcessBase::Get_Current_Thread_Time -- gets the current execution time in seconds

		Returns: current execution time
					
**********************************************************************************************************************/
double CVirtualProcessBase::Get_Current_Thread_Time( void ) const
{
	return CurrentTimeSeconds;
}

/**********************************************************************************************************************
	CVirtualProcessBase::Get_Reschedule_Interval -- gets the reschedule interval in seconds

		Returns: reschedule interval
					
**********************************************************************************************************************/
double CVirtualProcessBase::Get_Reschedule_Interval( void ) const
{
	return .1;
}

/**********************************************************************************************************************
	CVirtualProcessBase::Get_Reschedule_Time -- gets the execution time that this process should be run again at, in seconds

		Returns: next execution time
					
**********************************************************************************************************************/
double CVirtualProcessBase::Get_Reschedule_Time( void ) const
{
	double next_task_time = TaskScheduler->Get_Next_Task_Time();
	return std::min( CurrentTimeSeconds + Get_Reschedule_Interval(), next_task_time );
}

/**********************************************************************************************************************
	CVirtualProcessBase::Service -- recurrent execution logic

		current_time_seconds -- the current time in seconds
		context -- the tbb context that this process is being run under
					
**********************************************************************************************************************/
void CVirtualProcessBase::Service( double current_time_seconds, const CVirtualProcessExecutionContext & /*context*/ )
{
	if ( State == EVPS_INITIALIZING )
	{
		State = EVPS_RUNNING;
		FirstServiceTimeSeconds = current_time_seconds;
	}

	CurrentTimeSeconds = current_time_seconds;

	if ( Is_Shutting_Down() )
	{
		return;
	}

	// message and scheduled task logic
	Service_Message_Frames();

	TaskScheduler->Service( Get_Current_Thread_Time() );

	FATAL_ASSERT( !( Should_Reschedule() && Is_Shutting_Down() ) );

	if ( Should_Reschedule() )
	{
		Send_Virtual_Process_Message( EVirtualProcessID::CONCURRENCY_MANAGER, shared_ptr< const IVirtualProcessMessage >( new CRescheduleVirtualProcessMessage( Get_Reschedule_Time() ) ) );
	}

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
	CVirtualProcessBase::Service_Message_Frames -- handles all incoming virtual process messages
					
**********************************************************************************************************************/
void CVirtualProcessBase::Service_Message_Frames( void )
{
	if ( MyMailbox.get() == nullptr )
	{
		return;
	}

	// get all the queued incoming messages
	std::vector< shared_ptr< CVirtualProcessMessageFrame > > frames;
	MyMailbox->Remove_Frames( frames );

	// iterate each frame
	for ( uint32 i = 0; i < frames.size(); ++i )
	{
		const shared_ptr< CVirtualProcessMessageFrame > &frame = frames[ i ];
		EVirtualProcessID::Enum source_process_id = frame->Get_Process_ID();

		// iterate each message within the frame
		for ( auto iter = frame->Get_Frame_Begin(); iter != frame->Get_Frame_End(); ++iter )
		{
			Handle_Message( source_process_id, *iter );
		}
	}
}

/**********************************************************************************************************************
	CVirtualProcessBase::Handle_Shutdown_Mailboxes -- services all mailboxes that are pending shutdown, either erasing
		an associated outbound frame if we don't have the mailbox, or erasing the mailbox.  Notifies the manager
		that each mailbox has been released.
					
**********************************************************************************************************************/
void CVirtualProcessBase::Handle_Shutdown_Mailboxes( void )
{
	for ( auto iter = ShutdownMailboxes.cbegin(); iter != ShutdownMailboxes.cend(); ++iter )
	{
		EVirtualProcessID::Enum process_id = *iter;

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
		Send_Virtual_Process_Message( EVirtualProcessID::CONCURRENCY_MANAGER, shared_ptr< const IVirtualProcessMessage >( new CReleaseMailboxResponse( process_id ) ) );
	}

	ShutdownMailboxes.clear();
}

/**********************************************************************************************************************
	CVirtualProcessBase::Should_Reschedule -- should this task be rescheduled

		Returns: true if it should be rescheduled, otherwise false
					
**********************************************************************************************************************/
bool CVirtualProcessBase::Should_Reschedule( void ) const
{
	return State == EVPS_RUNNING;
}

/**********************************************************************************************************************
	CVirtualProcessBase::Handle_Message -- central message handling dispatcher

		key -- message sender
		message -- the thread message to handle
					
**********************************************************************************************************************/
void CVirtualProcessBase::Handle_Message( EVirtualProcessID::Enum process_id, const shared_ptr< const IVirtualProcessMessage > &message )
{
	const IVirtualProcessMessage *msg_base = message.get();

	Loki::TypeInfo hash_key( typeid( *msg_base ) );
	auto iter = MessageHandlers.find( hash_key );
	FATAL_ASSERT( iter != MessageHandlers.end() );

	iter->second->Handle_Message( process_id, message );
}

/**********************************************************************************************************************
	CVirtualProcessBase::Register_Message_Handlers -- creates message handlers for each message that we want to receive
					
**********************************************************************************************************************/
void CVirtualProcessBase::Register_Message_Handlers( void )
{
	REGISTER_THIS_HANDLER( CAddMailboxMessage, CVirtualProcessBase, Handle_Add_Mailbox_Message )
	REGISTER_THIS_HANDLER( CReleaseMailboxRequest, CVirtualProcessBase, Handle_Release_Mailbox_Request )
	REGISTER_THIS_HANDLER( CShutdownSelfRequest, CVirtualProcessBase, Handle_Shutdown_Self_Request )
} 

/**********************************************************************************************************************
	CVirtualProcessBase::Register_Handler -- registers a message handlers for a virtual process message

		message_type_info -- the C++ type of the message class
		handler -- message handling delegate
					
**********************************************************************************************************************/
void CVirtualProcessBase::Register_Handler( const std::type_info &message_type_info, const shared_ptr< IVirtualProcessMessageHandler > &handler )
{
	Loki::TypeInfo key( message_type_info );

	FATAL_ASSERT( MessageHandlers.find( key ) == MessageHandlers.end() );

	MessageHandlers[ key ] = handler;
}

/**********************************************************************************************************************
	CVirtualProcessBase::Handle_Add_Mailbox_Message -- handles the AddMailboxMessage message

		source_process_id -- id of the process source of the message
		message -- the AddMailboxMessage message
					
**********************************************************************************************************************/
void CVirtualProcessBase::Handle_Add_Mailbox_Message( EVirtualProcessID::Enum /*source_process_id*/, const shared_ptr< const CAddMailboxMessage > &message )
{
	EVirtualProcessID::Enum add_id = message->Get_Mailbox()->Get_Process_ID();
	FATAL_ASSERT( add_id != EVirtualProcessID::CONCURRENCY_MANAGER && add_id != EVirtualProcessID::LOGGING );

	if ( Mailboxes.find( add_id ) == Mailboxes.end() )
	{
		Mailboxes.insert( MailboxTableType::value_type( add_id, message->Get_Mailbox() ) );

		const SProcessProperties &properties = message->Get_Mailbox()->Get_Properties();
		IDToPropertiesTable.insert( IDToProcessPropertiesTableType::value_type( add_id, properties ) );
		PropertiesToIDTable.insert( ProcessPropertiesToIDTableType::value_type( properties, add_id ) );
	}
}

/**********************************************************************************************************************
	CVirtualProcessBase::Handle_Release_Mailbox_Request -- handles the ReleaseMailboxRequest message

		source_process_id -- id of the process source of the message
		message -- message to handle
					
**********************************************************************************************************************/
void CVirtualProcessBase::Handle_Release_Mailbox_Request( EVirtualProcessID::Enum source_process_id, const shared_ptr< const CReleaseMailboxRequest > &request )
{
	FATAL_ASSERT( source_process_id == EVirtualProcessID::CONCURRENCY_MANAGER );

	EVirtualProcessID::Enum shutdown_process_id = request->Get_Process_ID();
	FATAL_ASSERT( shutdown_process_id != EVirtualProcessID::CONCURRENCY_MANAGER && shutdown_process_id != EVirtualProcessID::LOGGING );

	ShutdownMailboxes.insert( shutdown_process_id );
}

/**********************************************************************************************************************
	CVirtualProcessBase::Handle_Shutdown_Self_Request -- handles the ShutdownSelf request

		source_process_id -- id of the process source of the message
		message -- the ShutdownThread request
					
**********************************************************************************************************************/
void CVirtualProcessBase::Handle_Shutdown_Self_Request( EVirtualProcessID::Enum source_process_id, const shared_ptr< const CShutdownSelfRequest > &message )
{
	FATAL_ASSERT( source_process_id == EVirtualProcessID::CONCURRENCY_MANAGER );
	FATAL_ASSERT( !Is_Shutting_Down() );

	if ( message->Get_Is_Hard_Shutdown() )
	{
		State = EVPS_SHUTTING_DOWN_HARD;
	}
	else
	{
		State = EVPS_SHUTTING_DOWN_SOFT;
	}

	Send_Virtual_Process_Message( EVirtualProcessID::CONCURRENCY_MANAGER, shared_ptr< const IVirtualProcessMessage >( new CShutdownSelfResponse() ) );	
}

/**********************************************************************************************************************
	CVirtualProcessBase::Is_Shutting_Down -- is this virtual process in the process of shutting down?

		Returns: true if shutting down, false otherwise
					
**********************************************************************************************************************/
bool CVirtualProcessBase::Is_Shutting_Down( void ) const
{
	return State == EVPS_SHUTTING_DOWN_SOFT || State == EVPS_SHUTTING_DOWN_HARD;
}

/**********************************************************************************************************************
	CVirtualProcessBase::Flush_System_Messages -- Manager and log messages get sent separately from other messages.
		This function must be the last function called in this thread's execution context/tbb-execute.  The instant a reschedule
		message is pushed to the manager, this thread may end up getting reexecuted which would cause data corruption
		if the current execution is still ongoing.
					
**********************************************************************************************************************/
void CVirtualProcessBase::Flush_System_Messages( void )
{
	bool is_shutting_down = Is_Shutting_Down();

	// Flush logging messages if possible
	if ( LoggingMailbox.get() != nullptr && LogFrame.get() != nullptr )
	{
		shared_ptr< CVirtualProcessMessageFrame > log_frame( LogFrame );
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
		shared_ptr< CVirtualProcessMessageFrame > manager_frame( ManagerFrame );
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
	CVirtualProcessBase::Remove_Process_ID_From_Tables -- removes a process id from the pair of tables mapping
		process properties to/from process ids

		process_id -- id of the process to remove all references to
					
**********************************************************************************************************************/
void CVirtualProcessBase::Remove_Process_ID_From_Tables( EVirtualProcessID::Enum process_id )
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
	CVirtualProcessBase::Build_Process_ID_List_By_Properties -- builds a list of all known processes that match
		a property set

		properties -- properties to match against
		process_ids -- output vector of matching process ids
					
**********************************************************************************************************************/
void CVirtualProcessBase::Build_Process_ID_List_By_Properties( const SProcessProperties &properties, std::vector< EVirtualProcessID::Enum > &process_ids ) const
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

