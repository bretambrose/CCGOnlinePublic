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
#include "ThreadSubject.h"
#include "MailboxInterfaces.h"
#include "VirtualProcessConstants.h"
#include "VirtualProcessMessageFrame.h"
#include "Messaging/LoggingMessages.h"
#include "Messaging/VirtualProcessManagementMessages.h"
#include "Messaging/ExchangeMailboxMessages.h"

enum EVirtualProcessState
{
	EVPS_INITIALIZING,
	EVPS_RUNNING,
	EVPS_SHUTTING_DOWN_SOFT,
	EVPS_SHUTTING_DOWN_HARD
};

/**********************************************************************************************************************
	CVirtualProcessBase::CVirtualProcessBase -- constructor
	
		key -- the key for this virtual process
				
**********************************************************************************************************************/
CVirtualProcessBase::CVirtualProcessBase( const SThreadKey &key ) :
	BASECLASS(),
	Key( key ),
	State( EVPS_INITIALIZING ),
	PendingOutboundFrames(),
	ManagerFrame( nullptr ),
	LogFrame( nullptr ),
	Mailboxes(),
	ManagerMailbox( nullptr ),
	LogMailbox( nullptr ),
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
					
**********************************************************************************************************************/
void CVirtualProcessBase::Initialize( void )
{
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
	FATAL_ASSERT( Key.Get_Thread_Subject() != TS_LOGGING );

	Send_Virtual_Process_Message( LOG_THREAD_KEY, shared_ptr< const IVirtualProcessMessage >( new CLogRequestMessage( Key, message ) ) );
}

/**********************************************************************************************************************
	CVirtualProcessBase::Get_Thread_Interface -- gets the mailbox for a thread, if we have it

		key -- key of the thread to get a mailbox for
					
**********************************************************************************************************************/
shared_ptr< CWriteOnlyMailbox > CVirtualProcessBase::Get_Mailbox( const SThreadKey &key ) const
{
	auto interface_iter = Mailboxes.find( key );
	if ( interface_iter != Mailboxes.end() )
	{
		return interface_iter->second;
	}

	return shared_ptr< CWriteOnlyMailbox >( nullptr );
}

/**********************************************************************************************************************
	CVirtualProcessBase::Send_Virtual_Process_Message -- sends a message to another virtual process

		dest_key -- key of the virtual process to send to
		message -- message to send
					
**********************************************************************************************************************/
void CVirtualProcessBase::Send_Virtual_Process_Message( const SThreadKey &dest_key, const shared_ptr< const IVirtualProcessMessage > &message )
{
	// manager and logging threads are special-cased in order to avoid race conditions related to rescheduling
	if ( dest_key == MANAGER_THREAD_KEY )
	{
		if ( ManagerFrame.get() == nullptr )
		{
			ManagerFrame.reset( new CVirtualProcessMessageFrame( Key ) );
		}
		
		ManagerFrame->Add_Message( message );
	}
	else if ( dest_key == LOG_THREAD_KEY )
	{
		if ( LogFrame.get() == nullptr )
		{
			LogFrame.reset( new CVirtualProcessMessageFrame( Key ) );
		}
		
		LogFrame->Add_Message( message );
	}
	else
	{
		// find or create a frame for the destination thread
		auto iter = PendingOutboundFrames.find( dest_key );
		if ( iter == PendingOutboundFrames.end() )
		{
			shared_ptr< CVirtualProcessMessageFrame > frame( new CVirtualProcessMessageFrame( Get_Key() ) );
			frame->Add_Message( message );
			PendingOutboundFrames.insert( FrameTableType::value_type( dest_key, frame ) );
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
		std::vector< SThreadKey > sent_frames;

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
			if ( State == EVPS_SHUTTING_DOWN_SOFT || frame_iterator->first == MANAGER_THREAD_KEY || frame_iterator->first == LOG_THREAD_KEY )
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
		Send_Virtual_Process_Message( MANAGER_THREAD_KEY, shared_ptr< const IVirtualProcessMessage >( new CRescheduleVirtualProcessMessage( Key, Get_Reschedule_Time() ) ) );
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
		SThreadKey key = frame->Get_Key();

		// iterate each message within the frame
		for ( auto iter = frame->Get_Frame_Begin(); iter != frame->Get_Frame_End(); ++iter )
		{
			Handle_Message( key, *iter );
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
		const SThreadKey &key = *iter;

		auto interface_iter = Mailboxes.find( key );
		if ( interface_iter == Mailboxes.end() )
		{
			auto frame_iter = PendingOutboundFrames.find( key );
			if ( frame_iter != PendingOutboundFrames.end() )
			{
				PendingOutboundFrames.erase( frame_iter );
			}
		}
		else
		{
			// should have been sent in the flush that preceding this call
			FATAL_ASSERT( PendingOutboundFrames.find( key ) == PendingOutboundFrames.end() );

			Mailboxes.erase( interface_iter );
		}

		// let the manager know we've release this interface
		Send_Virtual_Process_Message( MANAGER_THREAD_KEY, shared_ptr< const IVirtualProcessMessage >( new CReleaseMailboxResponse( key ) ) );
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
void CVirtualProcessBase::Handle_Message( const SThreadKey &key, const shared_ptr< const IVirtualProcessMessage > &message )
{
	const IVirtualProcessMessage *msg_base = message.get();

	Loki::TypeInfo hash_key( typeid( *msg_base ) );
	auto iter = MessageHandlers.find( hash_key );
	FATAL_ASSERT( iter != MessageHandlers.end() );

	iter->second->Handle_Message( key, message );
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

		key -- process source of the message
		message -- the AddMailboxMessage message
					
**********************************************************************************************************************/
void CVirtualProcessBase::Handle_Add_Mailbox_Message( const SThreadKey & /*key*/, const shared_ptr< const CAddMailboxMessage > &message )
{
	SThreadKey add_key = message->Get_Key();
	if ( add_key == LOG_THREAD_KEY )
	{
		LogMailbox = message->Get_Mailbox();
	}
	else if ( Mailboxes.find( add_key ) == Mailboxes.end() )
	{
		Mailboxes.insert( MailboxTableType::value_type( add_key, message->Get_Mailbox() ) );
	}
}

/**********************************************************************************************************************
	CVirtualProcessBase::Handle_Release_Mailbox_Request -- handles the ReleaseMailboxRequest message

		key -- process source of the message
		message -- message to handle
					
**********************************************************************************************************************/
void CVirtualProcessBase::Handle_Release_Mailbox_Request( const SThreadKey &key, const shared_ptr< const CReleaseMailboxRequest > &request )
{
	FATAL_ASSERT( key == MANAGER_THREAD_KEY );

	SThreadKey shutdown_key = request->Get_Key();
	FATAL_ASSERT( shutdown_key != MANAGER_THREAD_KEY && shutdown_key != LOG_THREAD_KEY );

	ShutdownMailboxes.insert( shutdown_key );
}

/**********************************************************************************************************************
	CVirtualProcessBase::Handle_Shutdown_Self_Request -- handles the ShutdownSelf request

		key -- thread source of the message
		message -- the ShutdownThread request
					
**********************************************************************************************************************/
void CVirtualProcessBase::Handle_Shutdown_Self_Request( const SThreadKey &key, const shared_ptr< const CShutdownSelfRequest > &message )
{
	FATAL_ASSERT( key == MANAGER_THREAD_KEY );
	FATAL_ASSERT( !Is_Shutting_Down() );

	if ( message->Get_Is_Hard_Shutdown() )
	{
		State = EVPS_SHUTTING_DOWN_HARD;
	}
	else
	{
		State = EVPS_SHUTTING_DOWN_SOFT;
	}

	Send_Virtual_Process_Message( MANAGER_THREAD_KEY, shared_ptr< const IVirtualProcessMessage >( new CShutdownSelfResponse() ) );	
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
	if ( LogMailbox.get() != nullptr && LogFrame.get() != nullptr )
	{
		shared_ptr< CVirtualProcessMessageFrame > log_frame( LogFrame );
		LogFrame.reset();

		LogMailbox->Add_Frame( log_frame );
	}

	// Clear log interface if necessary
	if ( is_shutting_down )
	{
		LogMailbox.reset();
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