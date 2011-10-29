/**********************************************************************************************************************

	[Placeholder for eventual source license]

	ThreadTaskBase.cpp
		A component containing the logic shared by all thread tasks.

	(c) Copyright 2011, Bret Ambrose.  All rights reserved.

**********************************************************************************************************************/

#include "stdafx.h"

#include "ThreadTaskBase.h"

#include "MessageHandling/ThreadMessageHandler.h"
#include "TaskScheduler/TaskScheduler.h"
#include "ThreadSubject.h"
#include "ThreadInterfaces.h"
#include "ThreadConstants.h"
#include "ThreadMessageFrame.h"
#include "ThreadMessages/LoggingMessages.h"
#include "ThreadMessages/ThreadManagementMessages.h"
#include "ThreadMessages/ExchangeInterfaceMessages.h"

enum EThreadState
{
	ETS_INITIALIZING,
	ETS_RUNNING,
	ETS_SHUTTING_DOWN_SOFT,
	ETS_SHUTTING_DOWN_HARD
};

/**********************************************************************************************************************
	CThreadTaskBase::CThreadTaskBase -- constructor
	
		key -- the key for this thread task
				
**********************************************************************************************************************/
CThreadTaskBase::CThreadTaskBase( const SThreadKey &key ) :
	BASECLASS(),
	Key( key ),
	State( ETS_INITIALIZING ),
	PendingOutboundFrames(),
	ManagerFrame( nullptr ),
	LogFrame( nullptr ),
	Interfaces(),
	ManagerInterface( nullptr ),
	LogInterface( nullptr ),
	ReadInterface( nullptr ),
	ShutdownInterfaces(),
	FirstServiceTimeSeconds( 0.0 ),
	CurrentTimeSeconds( 0.0 ),
	MessageHandlers(),
	TaskScheduler( new CTaskScheduler )
{
}

/**********************************************************************************************************************
	CThreadTaskBase::~CThreadTaskBase -- destructor
					
**********************************************************************************************************************/
CThreadTaskBase::~CThreadTaskBase()
{
	Cleanup();
}

/**********************************************************************************************************************
	CThreadTaskBase::Initialize -- initializes the thread task
					
**********************************************************************************************************************/
void CThreadTaskBase::Initialize( void )
{
	Register_Message_Handlers();
}

/**********************************************************************************************************************
	CThreadTaskBase::Cleanup -- cleans up the thread task
					
**********************************************************************************************************************/
void CThreadTaskBase::Cleanup( void )
{
	MessageHandlers.clear();	// not really necessary
}

/**********************************************************************************************************************
	CThreadTaskBase::Log -- requests a message be written to the log file for this thread

		message -- message to output to the log file
					
**********************************************************************************************************************/
void CThreadTaskBase::Log( const std::wstring &message )
{
	// actual logging thread should override this function and never call the baseclass
	FATAL_ASSERT( Key.Get_Thread_Subject() != TS_LOGGING );

	Send_Thread_Message( LOG_THREAD_KEY, shared_ptr< const IThreadMessage >( new CLogRequestMessage( Key, message ) ) );
}

/**********************************************************************************************************************
	CThreadTaskBase::Get_Thread_Interface -- gets the interface for a thread, if we have it

		key -- key of the thread to get an interface for
					
**********************************************************************************************************************/
shared_ptr< CWriteOnlyThreadInterface > CThreadTaskBase::Get_Thread_Interface( const SThreadKey &key ) const
{
	auto interface_iter = Interfaces.find( key );
	if ( interface_iter != Interfaces.end() )
	{
		return interface_iter->second;
	}

	return shared_ptr< CWriteOnlyThreadInterface >( nullptr );
}

/**********************************************************************************************************************
	CThreadTaskBase::Send_Thread_Message -- sends a message to another thread

		dest_key -- key of the thread to send to
		message -- message to send
					
**********************************************************************************************************************/
void CThreadTaskBase::Send_Thread_Message( const SThreadKey &dest_key, const shared_ptr< const IThreadMessage > &message )
{
	// manager and logging threads are special-cased in order to avoid race conditions related to rescheduling
	if ( dest_key == MANAGER_THREAD_KEY )
	{
		if ( ManagerFrame.get() == nullptr )
		{
			ManagerFrame.reset( new CThreadMessageFrame( Key ) );
		}
		
		ManagerFrame->Add_Message( message );
	}
	else if ( dest_key == LOG_THREAD_KEY )
	{
		if ( LogFrame.get() == nullptr )
		{
			LogFrame.reset( new CThreadMessageFrame( Key ) );
		}
		
		LogFrame->Add_Message( message );
	}
	else
	{
		// find or create a frame for the destination thread
		auto iter = PendingOutboundFrames.find( dest_key );
		if ( iter == PendingOutboundFrames.end() )
		{
			shared_ptr< CThreadMessageFrame > frame( new CThreadMessageFrame( Get_Key() ) );
			frame->Add_Message( message );
			PendingOutboundFrames.insert( FrameTableType::value_type( dest_key, frame ) );
			return;
		}

		iter->second->Add_Message( message );
	}
}

/**********************************************************************************************************************
	CThreadTaskBase::Flush_Messages -- sends all pending messages to their destination thread; does not include
		manager or logging thread
					
**********************************************************************************************************************/
void CThreadTaskBase::Flush_Messages( void )
{
	if ( !Is_Shutting_Down() )
	{
		// under normal circumstances, send all messages to threads we have an interface for
		std::vector< SThreadKey > sent_frames;

		for ( auto frame_iterator = PendingOutboundFrames.cbegin(); frame_iterator != PendingOutboundFrames.cend(); ++frame_iterator )
		{
			shared_ptr< CWriteOnlyThreadInterface > write_interface = Get_Thread_Interface( frame_iterator->first );
			if ( write_interface != nullptr )
			{
				write_interface->Add_Frame( frame_iterator->second );
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
			if ( State == ETS_SHUTTING_DOWN_SOFT || frame_iterator->first == MANAGER_THREAD_KEY || frame_iterator->first == LOG_THREAD_KEY )
			{
				shared_ptr< CWriteOnlyThreadInterface > write_interface = Get_Thread_Interface( frame_iterator->first );
				if ( write_interface != nullptr )
				{
					write_interface->Add_Frame( frame_iterator->second );
				}
			}
		}

		// unlike normal operations, when shutting down, we remove all pending messages whether or not they were sent
		PendingOutboundFrames.clear();
	}
}

/**********************************************************************************************************************
	CThreadTaskBase::Set_Manager_Interface -- sets the write-only interface to the concurrency manager

		write_interface -- the manager's write interface
					
**********************************************************************************************************************/
void CThreadTaskBase::Set_Manager_Interface( const shared_ptr< CWriteOnlyThreadInterface > &write_interface )
{
	FATAL_ASSERT( ManagerInterface.get() == nullptr );

	ManagerInterface = write_interface;
}

/**********************************************************************************************************************
	CThreadTaskBase::Set_Read_Interface -- sets the read-only interface to ourself

		read_interface -- our read interface
					
**********************************************************************************************************************/
void CThreadTaskBase::Set_Read_Interface( const shared_ptr< CReadOnlyThreadInterface > &read_interface )
{
	ReadInterface = read_interface;
}

/**********************************************************************************************************************
	CThreadTaskBase::Get_Elapsed_Seconds -- gets how many seconds have elapsed since this thread started executing

		Returns: time in seconds that this thread has been executing
					
**********************************************************************************************************************/
double CThreadTaskBase::Get_Elapsed_Seconds( void ) const
{
	return CurrentTimeSeconds - FirstServiceTimeSeconds;
}

/**********************************************************************************************************************
	CThreadTaskBase::Get_Current_Thread_Time -- gets the current execution time in seconds

		Returns: current execution time
					
**********************************************************************************************************************/
double CThreadTaskBase::Get_Current_Thread_Time( void ) const
{
	return CurrentTimeSeconds;
}

/**********************************************************************************************************************
	CThreadTaskBase::Get_Reschedule_Interval -- gets the reschedule interval in seconds

		Returns: reschedule interval
					
**********************************************************************************************************************/
double CThreadTaskBase::Get_Reschedule_Interval( void ) const
{
	return .1;
}

/**********************************************************************************************************************
	CThreadTaskBase::Get_Reschedule_Time -- gets the execution time that this task should be run again at, in seconds

		Returns: next execution time
					
**********************************************************************************************************************/
double CThreadTaskBase::Get_Reschedule_Time( void ) const
{
	double next_task_time = TaskScheduler->Get_Next_Task_Time();
	return std::min( CurrentTimeSeconds + Get_Reschedule_Interval(), next_task_time );
}

/**********************************************************************************************************************
	CThreadTaskBase::Service -- recurrent execution logic

		current_time_seconds -- the current time in seconds
		context -- the tbb context that this task is being run under
					
**********************************************************************************************************************/
void CThreadTaskBase::Service( double current_time_seconds, const CThreadTaskExecutionContext & /*context*/ )
{
	if ( State == ETS_INITIALIZING )
	{
		State = ETS_RUNNING;
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
		Send_Thread_Message( MANAGER_THREAD_KEY, shared_ptr< const IThreadMessage >( new CRescheduleThreadMessage( Key, Get_Reschedule_Time() ) ) );
	}

	// Awkward but necessary due to shutdown sequence
	// We need to flush before we call Handle_Shutdown_Interfaces, but that function also needs to send messages that go out this moment too,
	// so we do before-and-after flushes
	Flush_Messages();
	Handle_Shutdown_Interfaces();
	Flush_Messages();

	if ( Is_Shutting_Down() )
	{
		FATAL_ASSERT( PendingOutboundFrames.size() == 0 );
		Interfaces.clear();
	}
}

/**********************************************************************************************************************
	CThreadTaskBase::Service_Message_Frames -- handles all incoming thread messages
					
**********************************************************************************************************************/
void CThreadTaskBase::Service_Message_Frames( void )
{
	if ( ReadInterface.get() == nullptr )
	{
		return;
	}

	// get all the queued incoming messages
	std::vector< shared_ptr< CThreadMessageFrame > > frames;
	ReadInterface->Remove_Frames( frames );

	// iterate each frame
	for ( uint32 i = 0; i < frames.size(); ++i )
	{
		const shared_ptr< CThreadMessageFrame > &frame = frames[ i ];
		SThreadKey key = frame->Get_Key();

		// iterate each message within the frame
		for ( auto iter = frame->Get_Frame_Begin(); iter != frame->Get_Frame_End(); ++iter )
		{
			Handle_Message( key, *iter );
		}
	}
}

/**********************************************************************************************************************
	CThreadTaskBase::Handle_Shutdown_Interfaces -- services all interfaces that are pending shutdown, either erasing
		an associated outbound frame if we don't have the interface, or erasing the interface.  Notifies the manager
		that each interface has been released.
					
**********************************************************************************************************************/
void CThreadTaskBase::Handle_Shutdown_Interfaces( void )
{
	for ( auto iter = ShutdownInterfaces.cbegin(); iter != ShutdownInterfaces.cend(); ++iter )
	{
		const SThreadKey &key = *iter;

		auto interface_iter = Interfaces.find( key );
		if ( interface_iter == Interfaces.end() )
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

			Interfaces.erase( interface_iter );
		}

		// let the manager know we've release this interface
		Send_Thread_Message( MANAGER_THREAD_KEY, shared_ptr< const IThreadMessage >( new CShutdownInterfaceAcknowledgement( key ) ) );
	}

	ShutdownInterfaces.clear();
}

/**********************************************************************************************************************
	CThreadTaskBase::Should_Reschedule -- should this task be rescheduled

		Returns: true if it should be rescheduled, otherwise false
					
**********************************************************************************************************************/
bool CThreadTaskBase::Should_Reschedule( void ) const
{
	return State == ETS_RUNNING;
}

/**********************************************************************************************************************
	CThreadTaskBase::Handle_Message -- central message handling dispatcher

		key -- message sender
		message -- the thread message to handle
					
**********************************************************************************************************************/
void CThreadTaskBase::Handle_Message( const SThreadKey &key, const shared_ptr< const IThreadMessage > &message )
{
	const IThreadMessage *msg_base = message.get();

	Loki::TypeInfo hash_key( typeid( *msg_base ) );
	auto iter = MessageHandlers.find( hash_key );
	FATAL_ASSERT( iter != MessageHandlers.end() );

	iter->second->Handle_Message( key, message );
}

/**********************************************************************************************************************
	CThreadTaskBase::Register_Message_Handlers -- creates message handlers for each message that we want to receive
					
**********************************************************************************************************************/
void CThreadTaskBase::Register_Message_Handlers( void )
{
	REGISTER_THIS_HANDLER( CAddInterfaceMessage, CThreadTaskBase, Handle_Add_Write_Interface_Message )
	REGISTER_THIS_HANDLER( CShutdownInterfaceMessage, CThreadTaskBase, Handle_Shutdown_Interface_Message )
	REGISTER_THIS_HANDLER( CShutdownThreadRequest, CThreadTaskBase, Handle_Shutdown_Thread_Request )
} 

/**********************************************************************************************************************
	CThreadTaskBase::Register_Handler -- registers a message handlers for a thread message

		message_type_info -- the C++ type of the message class
		handler -- message handling delegate
					
**********************************************************************************************************************/
void CThreadTaskBase::Register_Handler( const std::type_info &message_type_info, const shared_ptr< IThreadMessageHandler > &handler )
{
	Loki::TypeInfo key( message_type_info );

	FATAL_ASSERT( MessageHandlers.find( key ) == MessageHandlers.end() );

	MessageHandlers[ key ] = handler;
}

/**********************************************************************************************************************
	CThreadTaskBase::Handle_Add_Write_Interface_Message -- handles the AddWriteInterface message

		key -- thread source of the message
		message -- the AddWriteInterface message
					
**********************************************************************************************************************/
void CThreadTaskBase::Handle_Add_Write_Interface_Message( const SThreadKey & /*key*/, const shared_ptr< const CAddInterfaceMessage > &message )
{
	SThreadKey add_key = message->Get_Key();
	if ( add_key == LOG_THREAD_KEY )
	{
		LogInterface = message->Get_Interface();
	}
	else if ( Interfaces.find( add_key ) == Interfaces.end() )
	{
		Interfaces.insert( InterfaceTable::value_type( add_key, message->Get_Interface() ) );
	}
}

/**********************************************************************************************************************
	CThreadTaskBase::Handle_Shutdown_Interface_Message -- handles the ShutdownInterface message

		key -- thread source of the message
		message -- the ShutdownInterface message
					
**********************************************************************************************************************/
void CThreadTaskBase::Handle_Shutdown_Interface_Message( const SThreadKey &key, const shared_ptr< const CShutdownInterfaceMessage > &message )
{
	FATAL_ASSERT( key == MANAGER_THREAD_KEY );

	SThreadKey shutdown_key = message->Get_Key();
	FATAL_ASSERT( shutdown_key != MANAGER_THREAD_KEY && shutdown_key != LOG_THREAD_KEY );

	ShutdownInterfaces.insert( shutdown_key );
}

/**********************************************************************************************************************
	CThreadTaskBase::Handle_Shutdown_Thread_Request -- handles the ShutdownThread request

		key -- thread source of the message
		message -- the ShutdownThread request
					
**********************************************************************************************************************/
void CThreadTaskBase::Handle_Shutdown_Thread_Request( const SThreadKey &key, const shared_ptr< const CShutdownThreadRequest > &message )
{
	FATAL_ASSERT( key == MANAGER_THREAD_KEY );
	FATAL_ASSERT( !Is_Shutting_Down() );

	if ( message->Get_Is_Hard_Shutdown() )
	{
		State = ETS_SHUTTING_DOWN_HARD;
	}
	else
	{
		State = ETS_SHUTTING_DOWN_SOFT;
	}

	Send_Thread_Message( MANAGER_THREAD_KEY, shared_ptr< const IThreadMessage >( new CShutdownThreadAcknowledgement() ) );	
}

/**********************************************************************************************************************
	CThreadTaskBase::Is_Shutting_Down -- is this thread in the process of shutting down?

		Returns: true if shutting down, false otherwise
					
**********************************************************************************************************************/
bool CThreadTaskBase::Is_Shutting_Down( void ) const
{
	return State == ETS_SHUTTING_DOWN_SOFT || State == ETS_SHUTTING_DOWN_HARD;
}

/**********************************************************************************************************************
	CThreadTaskBase::Flush_Partitioned_Messages -- Manager and log messages get sent separately from other messages.
		This function must be the last function called in this thread's execution context/tbb-execute.  The instant a reschedule
		message is pushed to the manager, this thread may end up getting reexecuted which would cause data corruption
		if the current execution is still ongoing.
					
**********************************************************************************************************************/
void CThreadTaskBase::Flush_Partitioned_Messages( void )
{
	bool is_shutting_down = Is_Shutting_Down();

	// Flush logging messages if possible
	if ( LogInterface.get() != nullptr && LogFrame.get() != nullptr )
	{
		shared_ptr< CThreadMessageFrame > log_frame( LogFrame );
		LogFrame.reset();

		LogInterface->Add_Frame( log_frame );
	}

	// Clear log interface if necessary
	if ( is_shutting_down )
	{
		LogInterface.reset();
	}

	// Flush manager messages if possible
	if ( ManagerInterface.get() != nullptr && ManagerFrame.get() != nullptr )
	{
		shared_ptr< CThreadMessageFrame > manager_frame( ManagerFrame );
		ManagerFrame.reset();

		ManagerInterface->Add_Frame( manager_frame );
	}

	// Clear manager interface if necessary
	if ( is_shutting_down )
	{
		// logically safe to do even after we've pushed messages
		// A thread should not be rescheduled if it's in the shut down stage, and even it did
		// get rescheduled, the subsequent execution of the thread does nothing and does not access
		// the manager interface
		ManagerInterface.reset();	
	}
}