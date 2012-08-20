/**********************************************************************************************************************

	ConcurrencyManager.cpp
		A component definining the central, manager class that manages all virtual processes

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

#include "ConcurrencyManager.h"

#include "Logging/LogInterface.h"
#include "ManagedVirtualProcessInterface.h"
#include "ThreadSubject.h"
#include "VirtualProcessMailbox.h"
#include "MailboxInterfaces.h"
#include "VirtualProcessConstants.h"
#include "ThreadKeyManager.h"
#include "Messaging/ExchangeMailboxMessages.h"
#include "Messaging/LoggingMessages.h"
#include "Messaging/VirtualProcessManagementMessages.h"
#include "MessageHandling/VirtualProcessMessageHandler.h"
#include "VirtualProcessMessageFrame.h"
#include "TaskScheduler/TaskScheduler.h"
#include "TaskScheduler/ScheduledTask.h"
#include "Time/TimeKeeper.h"
#include "Time/TimeType.h"
#include "Time/TimeUtils.h"
#include "Time/TickTime.h"
#include "tbb/task_scheduler_init.h"
#include "tbb/task.h"
#include "VirtualProcessStatics.h"
#include "VirtualProcessExecutionContext.h"
#include "PlatformTime.h"
#include "PlatformProcess.h"


typedef FastDelegate2< const SThreadKey &, double, void > ExecuteProcessDelegateType;

// A scheduled task that triggers the execution of a scheduled process's service function by TBB
class CExecuteProcessScheduledTask : public CScheduledTask
{
	public:

		typedef CScheduledTask BASECLASS;

		CExecuteProcessScheduledTask( const ExecuteProcessDelegateType &execute_delegate, const SThreadKey &key, double execute_time_seconds ) :
			BASECLASS( execute_time_seconds ),
			ExecuteDelegate( execute_delegate ),
			Key( key )
		{}

		const SThreadKey &Get_Key( void ) const { return Key; }

		virtual bool Execute( double current_time_seconds, double & /*reschedule_time_seconds*/ )
		{
			ExecuteDelegate( Key, current_time_seconds );

			return false;
		}

	private:

		ExecuteProcessDelegateType ExecuteDelegate;
		SThreadKey Key;
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// A TBB task that executes a process's service function
class CServiceProcessTBBTask : public tbb::task
{
	public:

		typedef tbb::task BASECLASS;

		CServiceProcessTBBTask( const shared_ptr< IManagedVirtualProcess > &process, double elapsed_seconds ) :
			Process( process ),
			ElapsedSeconds( elapsed_seconds )
		{}

		virtual ~CServiceProcessTBBTask() {}

		virtual tbb::task *execute( void )
		{
			CVirtualProcessExecutionContext context( this );

			FATAL_ASSERT( CVirtualProcessStatics::Get_Current_Virtual_Process() == nullptr );

			CVirtualProcessStatics::Set_Current_Virtual_Process( Process.get() );
			Process->Service( ElapsedSeconds, context );
			CVirtualProcessStatics::Set_Current_Virtual_Process( nullptr );
			Process->Flush_System_Messages();

			return nullptr;
		}

	private:

		shared_ptr< IManagedVirtualProcess > Process;

		double ElapsedSeconds;
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// A TBB task that executes the logging thread's service function
class CServiceLoggingThreadTBBTask : public tbb::task
{
	public:

		typedef tbb::task BASECLASS;

		CServiceLoggingThreadTBBTask( double elapsed_seconds ) :
			ElapsedSeconds( elapsed_seconds )
		{}

		virtual ~CServiceLoggingThreadTBBTask() {}

		virtual tbb::task *execute( void )
		{
			CVirtualProcessExecutionContext context( this );

			FATAL_ASSERT( CVirtualProcessStatics::Get_Current_Virtual_Process() == nullptr );

			CLogInterface::Service_Logging( ElapsedSeconds, context );

			return nullptr;
		}

	private:

		double ElapsedSeconds;
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

enum EProcessState
{
	EPS_INITIALIZING,
	EPS_RUNNING,
	EPS_SHUTTING_DOWN_PHASE1,
	EPS_SHUTTING_DOWN_PHASE2
};

// An internal class that tracks state about a thread task
class CVirtualProcessRecord
{
	public:

		// Construction/destruction
		CVirtualProcessRecord( const shared_ptr< IManagedVirtualProcess > &process, const ExecuteProcessDelegateType &execute_delegate );
		CVirtualProcessRecord( const SThreadKey &key );
		~CVirtualProcessRecord();

		// Accessors
		const SThreadKey &Get_Key( void ) const { return Process->Get_Key(); }

		shared_ptr< IManagedVirtualProcess > Get_Virtual_Process( void ) const { return Process; }

		EProcessState Get_State( void ) const { return State; }
		void Set_State( EProcessState state ) { State = state; }

		CVirtualProcessMailbox *Get_Mailbox( void ) const { return Mailbox.get(); }

		// Operations
		void Add_Execute_Task( const shared_ptr< CTaskScheduler > &task_scheduler, double execution_time );
		void Remove_Execute_Task( const shared_ptr< CTaskScheduler > &task_scheduler );

		void Add_Pending_Shutdown_Key( const SThreadKey &key ) { PendingShutdownKeys.insert( key ); }
		void Remove_Pending_Shutdown_Key( const SThreadKey &key ) { PendingShutdownKeys.erase( key ); }

		// Queries
		bool Is_Shutting_Down( void ) const { return State == EPS_SHUTTING_DOWN_PHASE1 || State == EPS_SHUTTING_DOWN_PHASE2; }

		bool Has_Pending_Shutdown_Keys( void ) const { return PendingShutdownKeys.size() > 0; }

	private:

		SThreadKey Key;

		shared_ptr< IManagedVirtualProcess > Process;

		scoped_ptr< CVirtualProcessMailbox > Mailbox;

		shared_ptr< CExecuteProcessScheduledTask > ExecuteTask;

		ExecuteProcessDelegateType ExecuteDelegate;

		EProcessState State;

		std::set< SThreadKey, SThreadKeyContainerHelper > PendingShutdownKeys;
};

/**********************************************************************************************************************
	CVirtualProcessRecord::CVirtualProcessRecord -- constructor

		process -- pointer to the process this record tracks
		execute_delegate -- the function to invoke when it's time to execute the process through a TBB task
					
**********************************************************************************************************************/
CVirtualProcessRecord::CVirtualProcessRecord( const shared_ptr< IManagedVirtualProcess > &process, const ExecuteProcessDelegateType &execute_delegate ) :
	Key( process->Get_Key() ),
	Process( process ),
	Mailbox( new CVirtualProcessMailbox( process->Get_Key() ) ),
	ExecuteTask( nullptr ),
	ExecuteDelegate( execute_delegate ),
	State( EPS_INITIALIZING ),
	PendingShutdownKeys()
{
}

/**********************************************************************************************************************
	CVirtualProcessRecord::CVirtualProcessRecord -- alternative dummy constructor, only used to proxy the manager

		key -- key of the proxy process
					
**********************************************************************************************************************/
CVirtualProcessRecord::CVirtualProcessRecord( const SThreadKey &key ) :
	Key( key ),
	Process( nullptr ),
	Mailbox( new CVirtualProcessMailbox( key ) ),
	ExecuteTask( nullptr ),
	ExecuteDelegate(),
	State( EPS_INITIALIZING ),
	PendingShutdownKeys()
{
	FATAL_ASSERT( key == MANAGER_THREAD_KEY );
}

/**********************************************************************************************************************
	CVirtualProcessRecord::~CVirtualProcessRecord -- destructor
					
**********************************************************************************************************************/
CVirtualProcessRecord::~CVirtualProcessRecord()
{
	FATAL_ASSERT( ExecuteTask == nullptr || !ExecuteTask->Is_Scheduled() );
}

/**********************************************************************************************************************
	CVirtualProcessRecord::Add_Execute_Task -- Schedules an execution task for this process at a requested time

		task_scheduler -- task scheduler to schedule the task in
		execution_time -- time the process should be serviced
					
**********************************************************************************************************************/
void CVirtualProcessRecord::Add_Execute_Task( const shared_ptr< CTaskScheduler > &task_scheduler, double execution_time )
{
	if ( ExecuteTask == nullptr )
	{
		ExecuteTask.reset( new CExecuteProcessScheduledTask( ExecuteDelegate, Get_Key(), execution_time ) );
	}
	else
	{
		FATAL_ASSERT( !ExecuteTask->Is_Scheduled() );

		ExecuteTask->Set_Execute_Time( execution_time );
	}

	task_scheduler->Submit_Task( ExecuteTask );
}

/**********************************************************************************************************************
	CVirtualProcessRecord::Remove_Execute_Task -- removes a process's execution task from a task scheduler

		task_scheduler -- task scheduler to remove the task from
					
**********************************************************************************************************************/
void CVirtualProcessRecord::Remove_Execute_Task( const shared_ptr< CTaskScheduler > &task_scheduler )
{
	if ( ExecuteTask != nullptr && ExecuteTask->Is_Scheduled() )
	{
		task_scheduler->Remove_Task( ExecuteTask );
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

enum EConcurrencyManagerState
{
	ECMS_PRE_INITIALIZE,
	ECMS_INITIALIZED,
	ECMS_RUNNING,
	ECMS_SHUTTING_DOWN_PHASE1,
	ECMS_SHUTTING_DOWN_PHASE2,
	ECMS_FINISHED
};

/**********************************************************************************************************************
	CConcurrencyManager::CConcurrencyManager -- constructor
					
**********************************************************************************************************************/
CConcurrencyManager::CConcurrencyManager( void ) :
	ProcessRecords(),
	PersistentPushRequests(),
	UnfulfilledPushRequests(),
	PersistentGetRequests(),
	UnfulfilledGetRequests(),
	MessageHandlers(),
	PendingOutboundFrames(),
	TaskSchedulers(),
	TimeKeeper( new CTimeKeeper ),
	KeyManager( new CThreadKeyManager ),
	TBBTaskSchedulerInit( new tbb::task_scheduler_init ),
	State( ECMS_PRE_INITIALIZE )
{
	TaskSchedulers[ TT_REAL_TIME ] = shared_ptr< CTaskScheduler >( new CTaskScheduler );
	TaskSchedulers[ TT_GAME_TIME ] = shared_ptr< CTaskScheduler >( new CTaskScheduler );
}

/**********************************************************************************************************************
	CConcurrencyManager::~CConcurrencyManager -- destructor
					
**********************************************************************************************************************/
CConcurrencyManager::~CConcurrencyManager()
{
	Shutdown();
}

/**********************************************************************************************************************
	CConcurrencyManager::Initialize -- initializes the concurrency manager before running

		delete_all_logs -- should all log files be cleaned up during the init process?
					
**********************************************************************************************************************/
void CConcurrencyManager::Initialize( bool delete_all_logs )
{
	FATAL_ASSERT( State == ECMS_PRE_INITIALIZE );

	CVirtualProcessStatics::Set_Concurrency_Manager( this );
	CLogInterface::Initialize_Dynamic( delete_all_logs );

	Register_Message_Handlers();

	State = ECMS_INITIALIZED;
}

/**********************************************************************************************************************
	CConcurrencyManager::Shutdown -- cleans up the concurrency manager state
					
**********************************************************************************************************************/
void CConcurrencyManager::Shutdown( void )
{
	FATAL_ASSERT( State == ECMS_SHUTTING_DOWN_PHASE2 || State == ECMS_INITIALIZED || State == ECMS_PRE_INITIALIZE );

	MessageHandlers.clear();

	UnfulfilledPushRequests.clear();
	PersistentPushRequests.clear();
	UnfulfilledGetRequests.clear();

	FATAL_ASSERT( ProcessRecords.size() == 0 );

	CLogInterface::Shutdown_Dynamic();
	CVirtualProcessStatics::Set_Concurrency_Manager( nullptr );

	State = ECMS_FINISHED;
}

/**********************************************************************************************************************
	CConcurrencyManager::Get_Record -- gets the thread task record for a given thread key

		key -- key of the thread to get the task record for

		Returns: pointer to the thread task record, or null
					
**********************************************************************************************************************/
shared_ptr< CVirtualProcessRecord > CConcurrencyManager::Get_Record( const SThreadKey &key ) const
{
	auto iter = ProcessRecords.find( key );
	if ( iter != ProcessRecords.end() )
	{
		return iter->second;
	}

	return nullptr;
}

/**********************************************************************************************************************
	CConcurrencyManager::Get_Virtual_Process -- gets the process for a given thread key

		key -- key of the thread to get the process for

		Returns: pointer to the virtual process, or null
					
**********************************************************************************************************************/
shared_ptr< IManagedVirtualProcess > CConcurrencyManager::Get_Virtual_Process( const SThreadKey &key ) const
{
	shared_ptr< CVirtualProcessRecord > record = Get_Record( key );
	if ( record != nullptr )
	{
		return record->Get_Virtual_Process();
	}

	return shared_ptr< IManagedVirtualProcess >( nullptr );
}

/**********************************************************************************************************************
	CConcurrencyManager::Run -- starts the concurrency manager system

		starting_thread -- initial thread task
					
**********************************************************************************************************************/
void CConcurrencyManager::Run( const shared_ptr< IManagedVirtualProcess > &starting_process )
{
	Setup_For_Run( starting_process );

	Service();
}

/**********************************************************************************************************************
	CConcurrencyManager::Setup_For_Run -- prepares the manager to begin its service loop

		starting_process -- initial virtual process
					
**********************************************************************************************************************/
void CConcurrencyManager::Setup_For_Run( const shared_ptr< IManagedVirtualProcess > &starting_process )
{
	FATAL_ASSERT( State == ECMS_INITIALIZED );
	FATAL_ASSERT( ProcessRecords.size() == 0 );

	// make a proxy for the manager
	ProcessRecords[ MANAGER_THREAD_KEY ] = shared_ptr< CVirtualProcessRecord >( new CVirtualProcessRecord( MANAGER_THREAD_KEY ) );

	// all threads should receive an interface to the log thread automatically
	Handle_Push_Mailbox_Request( MANAGER_THREAD_KEY, shared_ptr< CPushMailboxRequest >( new CPushMailboxRequest( LOG_THREAD_KEY, ALL_THREAD_KEY ) ) );

	// Setup the logging thread and the initial thread
	Add_Virtual_Process( CLogInterface::Get_Logging_Process() );
	Add_Virtual_Process( shared_ptr< IManagedVirtualProcess>( starting_process ) );

	// Reset time
	TimeKeeper->Set_Base_Time( TT_REAL_TIME, STickTime( CPlatformTime::Get_High_Resolution_Time() ) );
	TimeKeeper->Set_Base_Time( TT_GAME_TIME, STickTime( 0 ) );

	State = ECMS_RUNNING;
}

/**********************************************************************************************************************
	CConcurrencyManager::Add_Virtual_Process -- adds a new virtual process into the concurrency system

		thread -- virtual process to add to the concurrency system
					
**********************************************************************************************************************/
void CConcurrencyManager::Add_Virtual_Process( const shared_ptr< IManagedVirtualProcess > &process )
{
	FATAL_ASSERT( process->Get_Key().Get_Thread_Subject() != TS_CONCURRENCY_MANAGER );

	// perform any sub key allocation if necessary
	SThreadKey new_key = KeyManager->Fill_In_Thread_Key( process->Get_Key() );
	FATAL_ASSERT( new_key.Is_Valid() );
	FATAL_ASSERT( ProcessRecords.find( new_key ) == ProcessRecords.end() );

	process->Set_Key( new_key );
	KeyManager->Add_Tracked_Thread_Key( new_key ); 

	process->Initialize();

	// track the thread task in a task record
	shared_ptr< CVirtualProcessRecord > process_record( new CVirtualProcessRecord( process, ExecuteProcessDelegateType( this, &CConcurrencyManager::Execute_Virtual_Process ) ) );
	ProcessRecords[ new_key ] = process_record;

	// Interface setup
	CVirtualProcessMailbox *mailbox = process_record->Get_Mailbox();
	Handle_Ongoing_Mailbox_Requests( mailbox );

	process->Set_Manager_Mailbox( Get_Mailbox( MANAGER_THREAD_KEY ) );
	process->Set_My_Mailbox( mailbox->Get_Readable_Mailbox() );

	// Schedule first execution if necessary
	if ( process->Is_Root_Thread() )
	{
		process_record->Add_Execute_Task( Get_Task_Scheduler( process->Get_Time_Type() ), TimeKeeper->Get_Elapsed_Seconds( process->Get_Time_Type() ) );
	}
}

/**********************************************************************************************************************
	CConcurrencyManager::Handle_Ongoing_Interface_Requests -- when a process is added, this function pushes all
		interfaces that should be given to the new process as well as sends this process's interface to everyone
		who has an outstanding request for its mailbox

		thread_connection -- handle to a process's read and write interfaces
					
**********************************************************************************************************************/
void CConcurrencyManager::Handle_Ongoing_Mailbox_Requests( CVirtualProcessMailbox *mailbox )
{
	const SThreadKey &new_key = mailbox->Get_Key();
	FATAL_ASSERT( !Is_Process_Shutting_Down( new_key ) );

	// persistent push requests
	for ( auto iter = PersistentPushRequests.cbegin(); iter != PersistentPushRequests.cend(); ++iter )
	{
		const shared_ptr< const CPushMailboxRequest > &push_request = *iter;
		const SThreadKey &source_key = push_request->Get_Source_Key();
		if ( push_request->Get_Target_Key().Matches( new_key ) && source_key != new_key && !Is_Process_Shutting_Down( source_key ) )
		{
			shared_ptr< const IVirtualProcessMessage > message( new CAddMailboxMessage( source_key, Get_Mailbox( source_key ) ) );
			Send_Virtual_Process_Message( new_key, message );
		}
	}

	// persistent get requests
	for ( auto iter = PersistentGetRequests.cbegin(); iter != PersistentGetRequests.cend(); ++iter )
	{
		const shared_ptr< const CGetMailboxRequest > &get_request = *iter;
		const SThreadKey &source_key = get_request->Get_Source_Key();
		if ( get_request->Get_Target_Key().Matches( new_key ) && source_key != new_key && !Is_Process_Shutting_Down( source_key ) )
		{
			shared_ptr< const IVirtualProcessMessage > message( new CAddMailboxMessage( new_key, Get_Mailbox( new_key ) ) );
			Send_Virtual_Process_Message( source_key, message );
		}
	}

	// unfulfilled single-targeted get requests
	auto push_range = UnfulfilledPushRequests.equal_range( new_key );
	for ( auto iter = push_range.first; iter != push_range.second; ++iter )
	{
		const shared_ptr< const CPushMailboxRequest > &push_request = iter->second;
		const SThreadKey &source_key = push_request->Get_Source_Key();

		if ( !Is_Process_Shutting_Down( source_key ) )
		{
			shared_ptr< const IVirtualProcessMessage > message( new CAddMailboxMessage( source_key, Get_Mailbox( source_key ) ) );
			Send_Virtual_Process_Message( new_key, message );
		}
	}

	UnfulfilledPushRequests.erase( push_range.first, push_range.second );

	// unfulfilled get requests
	auto get_range = UnfulfilledGetRequests.equal_range( new_key );
	for ( auto iter = get_range.first; iter != get_range.second; ++iter )
	{
		const shared_ptr< const CGetMailboxRequest > &get_request = iter->second;
		const SThreadKey &source_key = get_request->Get_Source_Key();

		if ( !Is_Process_Shutting_Down( source_key ) )
		{
			shared_ptr< const IVirtualProcessMessage > message( new CAddMailboxMessage( new_key, Get_Mailbox( new_key ) ) );
			Send_Virtual_Process_Message( source_key, message );
		}
	}

	UnfulfilledGetRequests.erase( get_range.first, get_range.second );
}

/**********************************************************************************************************************
	CConcurrencyManager::Get_Mailbox -- gets the write-only mailbox to a virtual process

		key -- key of the virtual process to get the mailbox for

		Returns: a pointer to the write-only mailbox, or null
					
**********************************************************************************************************************/
shared_ptr< CWriteOnlyMailbox > CConcurrencyManager::Get_Mailbox( const SThreadKey &key ) const
{
	auto iter = ProcessRecords.find( key );
	if ( iter != ProcessRecords.end() )
	{
		return iter->second->Get_Mailbox()->Get_Writable_Mailbox();
	}

	return shared_ptr< CWriteOnlyMailbox >( nullptr );
}

/**********************************************************************************************************************
	CConcurrencyManager::Get_My_Mailbox -- gets the read-only mailbox of the manager

		Returns: a pointer to the read-only mailbox of the manager
					
**********************************************************************************************************************/
shared_ptr< CReadOnlyMailbox > CConcurrencyManager::Get_My_Mailbox( void ) const
{
	auto iter = ProcessRecords.find( MANAGER_THREAD_KEY );
	FATAL_ASSERT( iter != ProcessRecords.end() );

	return iter->second->Get_Mailbox()->Get_Readable_Mailbox();
}

/**********************************************************************************************************************
	CConcurrencyManager::Send_Virtual_Process_Message -- queues up a message to a virtual process

		dest_key -- the message's destination process
		message -- message to send
					
**********************************************************************************************************************/
void CConcurrencyManager::Send_Virtual_Process_Message( const SThreadKey &dest_key, const shared_ptr< const IVirtualProcessMessage > &message )
{
	auto iter = PendingOutboundFrames.find( dest_key );
	if ( iter == PendingOutboundFrames.end() )
	{
		shared_ptr< CVirtualProcessMessageFrame > frame( new CVirtualProcessMessageFrame( MANAGER_THREAD_KEY ) );
		frame->Add_Message( message );
		PendingOutboundFrames.insert( FrameTableType::value_type( dest_key, frame ) );
		return;
	}

	iter->second->Add_Message( message );
}

/**********************************************************************************************************************
	CConcurrencyManager::Flush_Frames -- sends all outbound message frames to their respective process destinations
					
**********************************************************************************************************************/
void CConcurrencyManager::Flush_Frames( void )
{
	std::vector< SThreadKey > sent_frames;

	for ( auto frame_iterator = PendingOutboundFrames.cbegin(); frame_iterator != PendingOutboundFrames.cend(); ++frame_iterator )
	{
		shared_ptr< CWriteOnlyMailbox > write_interface = Get_Mailbox( frame_iterator->first );
		if ( write_interface != nullptr )
		{
			write_interface->Add_Frame( frame_iterator->second );
			sent_frames.push_back( frame_iterator->first );
		}
	}

	// only erase frames that were actually sent
	for ( uint32 i = 0; i < sent_frames.size(); i++ )
	{
		PendingOutboundFrames.erase( sent_frames[ i ] );
	}
}

/**********************************************************************************************************************
	CConcurrencyManager::Service -- run function for the concurrency system
					
**********************************************************************************************************************/
void CConcurrencyManager::Service( void )
{
	while ( ProcessRecords.size() > 0 )
	{
		Service_One_Iteration();

		NPlatform::Sleep( 0 );	// make this adjustable later depending on the service's needs
	}
}

/**********************************************************************************************************************
	CConcurrencyManager::Service_One_Iteration -- performs a single service iteration of the concurrency system
					
**********************************************************************************************************************/
void CConcurrencyManager::Service_One_Iteration( void )
{
	Service_Incoming_Frames();
	Flush_Frames();

	TimeKeeper->Set_Current_Time( TT_REAL_TIME, STickTime( CPlatformTime::Get_High_Resolution_Time() ) );
	// TODO game time updates here as needed

	for ( auto iter = TaskSchedulers.cbegin(); iter != TaskSchedulers.cend(); ++iter )
	{
		Get_Task_Scheduler( iter->first )->Service( TimeKeeper->Get_Elapsed_Seconds( iter->first ) );	
	}

	Service_Shutdown();
}

/**********************************************************************************************************************
	CConcurrencyManager::Service_Shutdown -- handles shutdown-related logic and state transitions
					
**********************************************************************************************************************/
void CConcurrencyManager::Service_Shutdown( void )
{
	if ( ProcessRecords.size() == 2 && State != ECMS_SHUTTING_DOWN_PHASE2 )
	{
		// Nothing left but the log thread and our own proxy thread
		FATAL_ASSERT( Get_Record( MANAGER_THREAD_KEY ) != nullptr && Get_Record( LOG_THREAD_KEY ) != nullptr );

		State = ECMS_SHUTTING_DOWN_PHASE2;

		shared_ptr< const IVirtualProcessMessage > message( new CShutdownSelfRequest( true ) );
		Send_Virtual_Process_Message( LOG_THREAD_KEY, message );
	}
	else if ( ProcessRecords.size() == 1 )
	{
		// Log thread now gone, we're ready to quit
		FATAL_ASSERT( Get_Record( MANAGER_THREAD_KEY ) != nullptr );

		ProcessRecords.clear();
	}
}

/**********************************************************************************************************************
	CConcurrencyManager::Service_Incoming_Frames -- iterates all incoming message frames and handles the messages
		within them
					
**********************************************************************************************************************/
void CConcurrencyManager::Service_Incoming_Frames( void )
{
	std::vector< shared_ptr< CVirtualProcessMessageFrame > > control_frames;
	Get_My_Mailbox()->Remove_Frames( control_frames );

	// iterate all frames
	for ( uint32 i = 0; i < control_frames.size(); ++i )
	{
		const shared_ptr< CVirtualProcessMessageFrame > &frame = control_frames[ i ];
		const SThreadKey &key = frame->Get_Key();

		// iterate all messages in the frame
		for ( auto iter = frame->Get_Frame_Begin(); iter != frame->Get_Frame_End(); ++iter )
		{
			Handle_Message( key, *iter );
		}
	}
}

/**********************************************************************************************************************
	CConcurrencyManager::Handle_Message -- top-level dispatch function for handling an incoming thread message

		key -- key of the message sender
		message -- message that was sent to the manager
					
**********************************************************************************************************************/
void CConcurrencyManager::Handle_Message( const SThreadKey &key, const shared_ptr< const IVirtualProcessMessage > &message )
{
	const IVirtualProcessMessage *msg_base = message.get();

	Loki::TypeInfo hash_key( typeid( *msg_base ) );
	auto iter = MessageHandlers.find( hash_key );
	FATAL_ASSERT( iter != MessageHandlers.end() );

	iter->second->Handle_Message( key, message );
}

/**********************************************************************************************************************
	CConcurrencyManager::Register_Message_Handlers -- registers all message handlers for the concurrency manager
					
**********************************************************************************************************************/
void CConcurrencyManager::Register_Message_Handlers( void )
{
	REGISTER_THIS_HANDLER( CGetMailboxRequest, CConcurrencyManager, Handle_Get_Mailbox_Request )
	REGISTER_THIS_HANDLER( CPushMailboxRequest, CConcurrencyManager, Handle_Push_Mailbox_Request )
	REGISTER_THIS_HANDLER( CAddNewVirtualProcessMessage, CConcurrencyManager, Handle_Add_New_Virtual_Process_Message )
	REGISTER_THIS_HANDLER( CShutdownVirtualProcessMessage, CConcurrencyManager, Handle_Shutdown_Virtual_Process_Message )
	REGISTER_THIS_HANDLER( CRescheduleVirtualProcessMessage, CConcurrencyManager, Handle_Reschedule_Virtual_Process_Message )
	REGISTER_THIS_HANDLER( CReleaseMailboxResponse, CConcurrencyManager, Handle_Release_Mailbox_Response )
	REGISTER_THIS_HANDLER( CShutdownSelfResponse, CConcurrencyManager, Handle_Shutdown_Self_Response )
	REGISTER_THIS_HANDLER( CShutdownManagerMessage, CConcurrencyManager, Handle_Shutdown_Manager_Message )
} 

/**********************************************************************************************************************
	CConcurrencyManager::Register_Handler -- registers a single message handler for a message type

		message_type_info -- C++ type info for the message class
		handler -- message handling delegate to invoke for this message type
					
**********************************************************************************************************************/
void CConcurrencyManager::Register_Handler( const std::type_info &message_type_info, const shared_ptr< IVirtualProcessMessageHandler > &handler )
{
	Loki::TypeInfo key( message_type_info );

	FATAL_ASSERT( MessageHandlers.find( key ) == MessageHandlers.end() );
	MessageHandlers[ key ] = handler;
}

/**********************************************************************************************************************
	CConcurrencyManager::Handle_Get_Mailbox_Request -- message handler for a GetMailbox request

		key -- thread source of the request
		message -- the get mailbox request
					
**********************************************************************************************************************/
void CConcurrencyManager::Handle_Get_Mailbox_Request( const SThreadKey & /*key*/, const shared_ptr< const CGetMailboxRequest > &message )
{
	// don't handle messages while shutting down
	if ( Is_Manager_Shutting_Down() )
	{
		return;
	}

	// don't give out interfaces when a thread is shutting down
	const SThreadKey &requested_key = message->Get_Target_Key();
	const SThreadKey &source_key = message->Get_Source_Key();
	if ( Is_Process_Shutting_Down( source_key ) )
	{
		return;
	}

	if ( requested_key.Is_Unique() )
	{
		// it's a single-targeted request
		shared_ptr< CVirtualProcessRecord > record = Get_Record( requested_key );
		if ( record == nullptr )
		{
			// not yet fulfillable, track it until it can be fulfilled
			UnfulfilledGetRequests.insert( GetRequestCollectionType::value_type( requested_key, message ) );
		}
		else if ( !record->Is_Shutting_Down() )
		{
			// fulfill the request
			shared_ptr< const IVirtualProcessMessage > message( new CAddMailboxMessage( requested_key, record->Get_Mailbox()->Get_Writable_Mailbox() ) );
			Send_Virtual_Process_Message( source_key, message );
		}
	}
	else
	{
		// it's a persistent pattern-matching request, match all existing threads and track against future adds
		for ( auto iter = ProcessRecords.cbegin(); iter != ProcessRecords.cend(); ++iter )
		{
			if ( message->Get_Target_Key().Matches( iter->first ) && !Is_Process_Shutting_Down( iter->first ) )
			{
				shared_ptr< const IVirtualProcessMessage > message( new CAddMailboxMessage( iter->first, Get_Mailbox( iter->first ) ) );
				Send_Virtual_Process_Message( source_key, message );
			}
		}

		PersistentGetRequests.push_back( message );
	}
}

/**********************************************************************************************************************
	CConcurrencyManager::Handle_Push_Mailbox_Request -- message handler for a PushMailbox request

		key -- thread source of the request
		message -- the push mailbox request
					
**********************************************************************************************************************/
void CConcurrencyManager::Handle_Push_Mailbox_Request( const SThreadKey &key, const shared_ptr< const CPushMailboxRequest > &message )
{
	// don't handle messages while shutting down
	if ( Is_Manager_Shutting_Down() )
	{
		return;
	}

	const SThreadKey &target_key = message->Get_Target_Key();
	const SThreadKey &source_key = message->Get_Source_Key();
	FATAL_ASSERT( key == source_key || key == MANAGER_THREAD_KEY );
	FATAL_ASSERT( source_key.Is_Unique() );

	// don't push threads that are shutting down
	if ( Is_Process_Shutting_Down( source_key ) )
	{
		return;
	}

	if ( target_key.Is_Unique() )
	{
		// single target request
		if ( ProcessRecords.find( target_key ) != ProcessRecords.end() )
		{
			// it's fulfillable now, do it if the target is not shutting down
			if ( !Is_Process_Shutting_Down( target_key ) )
			{
				shared_ptr< const IVirtualProcessMessage > response_message( new CAddMailboxMessage( source_key, Get_Mailbox( source_key ) ) );
				Send_Virtual_Process_Message( target_key, response_message );
			}
		}
		else
		{
			// wait til we can fulfill this request
			UnfulfilledPushRequests.insert( PushRequestCollectionType::value_type( target_key, message ) );
		}
	}
	else
	{
		// it's a pattern matching request
		for ( auto iter = ProcessRecords.cbegin(); iter != ProcessRecords.cend(); ++iter )
		{
			if ( target_key.Matches( iter->first ) && !Is_Process_Shutting_Down( iter->first ) )
			{
				shared_ptr< const IVirtualProcessMessage > response_message( new CAddMailboxMessage( source_key, Get_Mailbox( source_key ) ) );
				Send_Virtual_Process_Message( iter->first, response_message );
			}
		}

		PersistentPushRequests.push_back( message );
	}
}

/**********************************************************************************************************************
	CConcurrencyManager::Handle_Add_New_Virtual_Process_Message -- message handler for an AddNewVirtualProcess message

		key -- thread source of the message
		message -- the add process message
					
**********************************************************************************************************************/
void CConcurrencyManager::Handle_Add_New_Virtual_Process_Message( const SThreadKey &key, const shared_ptr< const CAddNewVirtualProcessMessage > &message )
{
	const SThreadKey &new_key = message->Get_Virtual_Process()->Get_Key();
	FATAL_ASSERT( !Is_Process_Shutting_Down( new_key ) );

	if ( Is_Manager_Shutting_Down() )
	{
		return;
	}

	Add_Virtual_Process( static_pointer_cast< IManagedVirtualProcess >( message->Get_Virtual_Process() ) );

	// return mailbox, push mailbox options
	if ( !Is_Process_Shutting_Down( key ) )
	{
		if ( message->Should_Return_Mailbox() )
		{
			shared_ptr< const IVirtualProcessMessage > response_message( new CAddMailboxMessage( new_key, Get_Mailbox( new_key ) ) );
			Send_Virtual_Process_Message( key, response_message );
		}

		if ( message->Should_Forward_Creator_Mailbox() )
		{
			shared_ptr< const IVirtualProcessMessage > response_message( new CAddMailboxMessage( key, Get_Mailbox( key ) ) );
			Send_Virtual_Process_Message( message->Get_Virtual_Process()->Get_Key(), response_message );
		}
	}
}

/**********************************************************************************************************************
	CConcurrencyManager::Handle_Shutdown_Virtual_Process_Message -- message handler for an ShutdownVirtualProcess message

		key -- process source of the message
		message -- the shutdown process message
					
**********************************************************************************************************************/
void CConcurrencyManager::Handle_Shutdown_Virtual_Process_Message( const SThreadKey & /*key*/, const shared_ptr< const CShutdownVirtualProcessMessage > &message )
{
	if ( Is_Manager_Shutting_Down() )
	{
		return;
	}

	Initiate_Process_Shutdown( message->Get_Key() );
}

/**********************************************************************************************************************
	CConcurrencyManager::Handle_Reschedule_Virtual_Process_Message -- message handler for a RescheduleVirtualProcess message

		key -- process source of the message
		message -- the reschedule virtual process message
					
**********************************************************************************************************************/
void CConcurrencyManager::Handle_Reschedule_Virtual_Process_Message( const SThreadKey & /*key*/, const shared_ptr< const CRescheduleVirtualProcessMessage > &message )
{
	const SThreadKey &rescheduled_key = message->Get_Key();
	shared_ptr< CVirtualProcessRecord > record = Get_Record( rescheduled_key );
	if ( record == nullptr )
	{
		return;
	}

	ETimeType time_type = record->Get_Virtual_Process()->Get_Time_Type();
	double execute_time = std::min( message->Get_Reschedule_Time(), TimeKeeper->Get_Elapsed_Seconds( time_type ) );
	record->Add_Execute_Task( Get_Task_Scheduler( time_type ), execute_time );
}

/**********************************************************************************************************************
	CConcurrencyManager::Handle_Release_Mailbox_Response -- message handler for a ReleaseMailbox response

		key -- process source of the response
		response -- the release mailbox response
					
**********************************************************************************************************************/
void CConcurrencyManager::Handle_Release_Mailbox_Response( const SThreadKey &key, const shared_ptr< const CReleaseMailboxResponse > &response )
{
	if ( Is_Manager_Shutting_Down() )
	{
		return;
	}

	const SThreadKey &shutdown_key = response->Get_Shutdown_Key();
	shared_ptr< CVirtualProcessRecord > record = Get_Record( shutdown_key );
	if ( record == nullptr )
	{
		return;
	}

	FATAL_ASSERT( record->Get_State() == EPS_SHUTTING_DOWN_PHASE1 );

	record->Remove_Pending_Shutdown_Key( key );
	if ( !record->Has_Pending_Shutdown_Keys() )
	{
		// we've heard back from everyone; no one has a handle to this thread anymore, so we can tell it to shut down
		record->Set_State( EPS_SHUTTING_DOWN_PHASE2 );

		shared_ptr< const IVirtualProcessMessage > shutdown_request( new CShutdownSelfRequest( false ) );
		Send_Virtual_Process_Message( shutdown_key, shutdown_request );
	}
}

/**********************************************************************************************************************
	CConcurrencyManager::Handle_Shutdown_Self_Response -- message handler for a ShutdownSelf response

		key -- process source of the response
		message -- the shutdown self response
					
**********************************************************************************************************************/
void CConcurrencyManager::Handle_Shutdown_Self_Response( const SThreadKey &key, const shared_ptr< const CShutdownSelfResponse > & /*message*/ )
{
	auto iter = ProcessRecords.find( key );
	FATAL_ASSERT( iter != ProcessRecords.end() );
	FATAL_ASSERT( iter->second->Get_State() == EPS_SHUTTING_DOWN_PHASE2 || Is_Manager_Shutting_Down() );

	ProcessRecords.erase( iter );

	KeyManager->Remove_Tracked_Thread_Key( key );
}

/**********************************************************************************************************************
	CConcurrencyManager::Handle_Shutdown_Manager_Message -- message handler for a ShutdownManager message

		key -- thread source of the message
		message -- the shutdown manager message
					
**********************************************************************************************************************/
void CConcurrencyManager::Handle_Shutdown_Manager_Message( const SThreadKey & /*key*/, const shared_ptr< const CShutdownManagerMessage > & /*message*/ )
{
	if ( Is_Manager_Shutting_Down() )
	{
		return;
	}

	State = ECMS_SHUTTING_DOWN_PHASE1;

	// tell everyone to shut down
	shared_ptr< const IVirtualProcessMessage > shutdown_thread_msg( new CShutdownSelfRequest( true ) );
	for ( auto iter = ProcessRecords.cbegin(); iter != ProcessRecords.cend(); ++iter )
	{
		if ( iter->first == MANAGER_THREAD_KEY || iter->first == LOG_THREAD_KEY )
		{
			continue;
		}

		Send_Virtual_Process_Message( iter->first, shutdown_thread_msg );
	}
}

/**********************************************************************************************************************
	CConcurrencyManager::Get_Task_Scheduler -- gets the task scheduler for a specified time type

		time_type -- time type to get the task scheduler for

		Returns: pointer to the task scheduler for that time type
					
**********************************************************************************************************************/
shared_ptr< CTaskScheduler > CConcurrencyManager::Get_Task_Scheduler( ETimeType time_type ) const
{
	return TaskSchedulers.find( time_type )->second;
}

/**********************************************************************************************************************
	CConcurrencyManager::Execute_Virtual_Process -- queues up a tbb task to execute the supplied process's service

		key -- key of the process to have executed
		current_time_seconds -- current time from the process's standpoint
					
**********************************************************************************************************************/
void CConcurrencyManager::Execute_Virtual_Process( const SThreadKey &key, double current_time_seconds )
{
	auto iter = ProcessRecords.find( key );
	if ( iter == ProcessRecords.end() )
	{
		return;
	}

	shared_ptr< CVirtualProcessRecord > record = iter->second;
	if ( record->Get_State() == EPS_INITIALIZING )
	{
		record->Set_State( EPS_RUNNING );
	}

	shared_ptr< IManagedVirtualProcess > thread_task_base = record->Get_Virtual_Process();

	if ( key == LOG_THREAD_KEY )
	{
		CServiceLoggingThreadTBBTask &tbb_task = *new( tbb::task::allocate_root() ) CServiceLoggingThreadTBBTask( current_time_seconds );
		tbb::task::enqueue( tbb_task );
	}
	else
	{
		CServiceProcessTBBTask &tbb_task = *new( tbb::task::allocate_root() ) CServiceProcessTBBTask( thread_task_base, current_time_seconds );
		tbb::task::enqueue( tbb_task );
	}
}

/**********************************************************************************************************************
	CConcurrencyManager::Initiate_Process_Shutdown -- guides a process into the shutdown procedire

		key -- key of the process to start shutting down
					
**********************************************************************************************************************/
void CConcurrencyManager::Initiate_Process_Shutdown( const SThreadKey &key )
{
	FATAL_ASSERT( key.Is_Unique() );
	FATAL_ASSERT( key != MANAGER_THREAD_KEY && key != LOG_THREAD_KEY );

	shared_ptr< CVirtualProcessRecord > shutdown_record = Get_Record( key );
	if ( shutdown_record == nullptr || shutdown_record->Is_Shutting_Down() )
	{
		return;
	}

	shutdown_record->Set_State( EPS_SHUTTING_DOWN_PHASE1 );

	// this message can be shared and broadcast
	shared_ptr< const IVirtualProcessMessage > release_mailbox_msg( new CReleaseMailboxRequest( key ) );
	for ( auto iter = ProcessRecords.cbegin(); iter != ProcessRecords.cend(); ++iter )
	{
		if ( key == iter->first || iter->first == MANAGER_THREAD_KEY || iter->first == LOG_THREAD_KEY )
		{
			continue;
		}

		shutdown_record->Add_Pending_Shutdown_Key( iter->first );
		Send_Virtual_Process_Message( iter->first, release_mailbox_msg );
	}

	// Remove all push/get requests related to this process
	Clear_Related_Mailbox_Requests( key );

	// Move to the next state if we're not waiting on any other process acknowledgements
	if ( !shutdown_record->Has_Pending_Shutdown_Keys() )
	{
		shutdown_record->Set_State( EPS_SHUTTING_DOWN_PHASE2 );

		shared_ptr< const IVirtualProcessMessage > shutdown_request( new CShutdownSelfRequest( false ) );
		Send_Virtual_Process_Message( key, shutdown_request );
	}
}

/**********************************************************************************************************************
	CConcurrencyManager::Clear_Related_Mailbox_Requests -- clears mailbox requests related to a thread

		key -- key of the virtual process to clear mailbox requests about
					
**********************************************************************************************************************/
void CConcurrencyManager::Clear_Related_Mailbox_Requests( const SThreadKey &key )
{
	uint32 removed = 0;

	// persistent push requests
	for ( auto iter = PersistentPushRequests.end(); iter > PersistentPushRequests.begin();  )
	{
		--iter;
		if ( ( *iter )->Get_Source_Key() == key )
		{
			removed++;
			std::swap( ( *iter ), *( PersistentPushRequests.end() - removed ) );
		}
	}

	PersistentPushRequests.resize( PersistentPushRequests.size() - removed );

	// persistent get requests
	removed = 0;
	for ( auto iter = PersistentGetRequests.end(); iter > PersistentGetRequests.begin(); )
	{
		--iter;
		if ( ( *iter )->Get_Source_Key() == key )
		{
			removed++;
			std::swap( ( *iter ), *( PersistentGetRequests.end() - removed ) );
		}
	}

	PersistentGetRequests.resize( PersistentGetRequests.size() - removed );

	// unfulfilled push requests
	bool removed_one = true;
	while ( removed_one )
	{
		removed_one = false;

		for ( auto iter = UnfulfilledPushRequests.begin(); iter != UnfulfilledPushRequests.end(); ++iter )
		{
			if ( iter->second->Get_Source_Key() == key || iter->second->Get_Target_Key() == key )
			{
				UnfulfilledPushRequests.erase( iter );
				removed_one = true;
				break;
			}
		}
	}

	// unfulfilled get requests
	removed_one = true;
	while ( removed_one )
	{
		removed_one = false;

		for ( auto iter = UnfulfilledGetRequests.begin(); iter != UnfulfilledGetRequests.end(); ++iter )
		{
			if ( iter->second->Get_Source_Key() == key || iter->second->Get_Target_Key() == key )
			{
				UnfulfilledGetRequests.erase( iter );
				removed_one = true;
				break;
			}
		}
	}
}

/**********************************************************************************************************************
	CConcurrencyManager::Is_Process_Shutting_Down -- asks if the supplied process is in the process of shutting down

		key -- key of the virtual process to check shutdown status of

		Returns: true if the process is shutting down, false otherwise
					
**********************************************************************************************************************/
bool CConcurrencyManager::Is_Process_Shutting_Down( const SThreadKey &key ) const
{
	shared_ptr< CVirtualProcessRecord > record = Get_Record( key );
	if ( record == nullptr )
	{
		return false;
	}

	return record->Is_Shutting_Down();
}

/**********************************************************************************************************************
	CConcurrencyManager::Is_Manager_Shutting_Down -- is the manager in the process of shutting down

		Returns: true if the manager is shutting down, false otherwise
					
**********************************************************************************************************************/
bool CConcurrencyManager::Is_Manager_Shutting_Down( void ) const
{
	return State == ECMS_SHUTTING_DOWN_PHASE1 || State == ECMS_SHUTTING_DOWN_PHASE2;
}

/**********************************************************************************************************************
	CConcurrencyManager::Log -- logs some text to the log file for the manager

		message -- message to log
					
**********************************************************************************************************************/
void CConcurrencyManager::Log( const std::wstring &message )
{
	if ( State != ECMS_SHUTTING_DOWN_PHASE2 )
	{
		Send_Virtual_Process_Message( LOG_THREAD_KEY, shared_ptr< const IVirtualProcessMessage >( new CLogRequestMessage( MANAGER_THREAD_KEY, message ) ) );
	}
}

/**********************************************************************************************************************
	CConcurrencyManager::Get_Game_Time -- gets the current game time of the manager

		Returns: the current game time
					
**********************************************************************************************************************/
double CConcurrencyManager::Get_Game_Time( void ) const
{
	return TimeKeeper->Get_Elapsed_Seconds( TT_GAME_TIME );
}

/**********************************************************************************************************************
	CConcurrencyManager::Set_Game_Time -- sets the current game time of the manager

		game_time_seconds -- the new current game time
					
**********************************************************************************************************************/
void CConcurrencyManager::Set_Game_Time( double game_time_seconds )
{
	TimeKeeper->Set_Current_Time( TT_GAME_TIME, NTimeUtils::Convert_Seconds_To_Game_Ticks( game_time_seconds ) );
}
