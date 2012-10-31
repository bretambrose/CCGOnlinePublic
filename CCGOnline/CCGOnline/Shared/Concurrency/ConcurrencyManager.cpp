/**********************************************************************************************************************

	ConcurrencyManager.cpp
		A component definining the central, manager class that manages all processes

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
#include "MailboxInterfaces.h"
#include "ManagedProcessInterface.h"
#include "MessageHandling/ProcessMessageHandler.h"
#include "Messaging/ExchangeMailboxMessages.h"
#include "Messaging/LoggingMessages.h"
#include "Messaging/ProcessManagementMessages.h"
#include "PlatformProcess.h"
#include "PlatformTime.h"
#include "PlatformThread.h"
#include "ProcessConstants.h"
#include "ProcessExecutionContext.h"
#include "ProcessExecutionMode.h"
#include "ProcessID.h"
#include "ProcessMailbox.h"
#include "ProcessMessageFrame.h"
#include "ProcessStatics.h"
#include "ProcessSubject.h"
#include "TaskScheduler/ScheduledTask.h"
#include "TaskScheduler/TaskScheduler.h"
#include "tbb/task.h"
#include "tbb/task_scheduler_init.h"
#include "Time/TickTime.h"
#include "Time/TimeKeeper.h"
#include "Time/TimeType.h"
#include "Time/TimeUtils.h"

typedef FastDelegate2< EProcessID::Enum, double, void > ExecuteProcessDelegateType;

// A scheduled task that triggers the execution of a scheduled process's service function by TBB
class CExecuteProcessScheduledTask : public CScheduledTask
{
	public:

		typedef CScheduledTask BASECLASS;

		CExecuteProcessScheduledTask( const ExecuteProcessDelegateType &execute_delegate, EProcessID::Enum process_id, double execute_time_seconds ) :
			BASECLASS( execute_time_seconds ),
			ExecuteDelegate( execute_delegate ),
			ProcessID( process_id )
		{}

		EProcessID::Enum Get_Process_ID( void ) const { return ProcessID; }

		virtual bool Execute( double current_time_seconds, double & /*reschedule_time_seconds*/ )
		{
			ExecuteDelegate( ProcessID, current_time_seconds );

			return false;
		}

	private:

		ExecuteProcessDelegateType ExecuteDelegate;
		EProcessID::Enum ProcessID;
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// A TBB task that executes a process's service function
class CServiceProcessTBBTask : public tbb::task
{
	public:

		typedef tbb::task BASECLASS;

		CServiceProcessTBBTask( const shared_ptr< IManagedProcess > &process, double elapsed_seconds ) :
			Process( process ),
			ElapsedSeconds( elapsed_seconds )
		{}

		virtual ~CServiceProcessTBBTask() {}

		virtual tbb::task *execute( void )
		{
			CProcessExecutionContext context( this, ElapsedSeconds );

			FATAL_ASSERT( CProcessStatics::Get_Current_Process() == nullptr );

			CProcessStatics::Set_Current_Process( Process.get() );
			Process->Run( context );
			CProcessStatics::Set_Current_Process( nullptr );
			Process->Flush_System_Messages();

			return nullptr;
		}

	private:

		shared_ptr< IManagedProcess > Process;

		double ElapsedSeconds;
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// A TBB task that executes the logging process's service function
class CServiceLoggingProcessTBBTask : public tbb::task
{
	public:

		typedef tbb::task BASECLASS;

		CServiceLoggingProcessTBBTask( double elapsed_seconds ) :
			ElapsedSeconds( elapsed_seconds )
		{}

		virtual ~CServiceLoggingProcessTBBTask() {}

		virtual tbb::task *execute( void )
		{
			CProcessExecutionContext context( this, ElapsedSeconds );

			FATAL_ASSERT( CProcessStatics::Get_Current_Process() == nullptr );

			CLogInterface::Service_Logging( context );

			return nullptr;
		}

	private:

		double ElapsedSeconds;
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

enum EInternalProcessState
{
	EIPS_INITIALIZING,
	EIPS_RUNNING,
	EIPS_SHUTTING_DOWN_PHASE1,
	EIPS_SHUTTING_DOWN_PHASE2
};

// An internal class that tracks state about a thread task
class CProcessRecord
{
	public:

		// Construction/destruction
		CProcessRecord( const shared_ptr< IManagedProcess > &process, const ExecuteProcessDelegateType &execute_delegate );
		CProcessRecord( EProcessID::Enum process_id );
		~CProcessRecord();

		// Accessors
		EProcessID::Enum Get_Process_ID( void ) const { return ProcessID; }
		SProcessProperties Get_Properties( void ) const {
			if ( ProcessID == EProcessID::CONCURRENCY_MANAGER )
			{
				return MANAGER_PROCESS_PROPERTIES;
			}
			else
			{
				return Process->Get_Properties();
			}
		}

		shared_ptr< IManagedProcess > Get_Process( void ) const { return Process; }

		EInternalProcessState Get_State( void ) const { return State; }
		void Set_State( EInternalProcessState state ) { State = state; }

		CProcessMailbox *Get_Mailbox( void ) const { return Mailbox.get(); }

		CPlatformThread *Get_Platform_Thread( void ) const { return PlatformThread.get(); }

		// Operations
		void Add_Execute_Task( const shared_ptr< CTaskScheduler > &task_scheduler, double execution_time );
		void Remove_Execute_Task( const shared_ptr< CTaskScheduler > &task_scheduler );

		void Add_Pending_Shutdown_PID( EProcessID::Enum process_id ) { PendingShutdownIDs.insert( process_id ); }
		void Remove_Pending_Shutdown_PID( EProcessID::Enum process_id ) { PendingShutdownIDs.erase( process_id ); }

		// Queries
		bool Is_Shutting_Down( void ) const { return State == EIPS_SHUTTING_DOWN_PHASE1 || State == EIPS_SHUTTING_DOWN_PHASE2; }

		bool Has_Pending_Shutdown_IDs( void ) const { return PendingShutdownIDs.size() > 0; }

	private:

		EProcessID::Enum ProcessID;

		shared_ptr< IManagedProcess > Process;

		unique_ptr< CProcessMailbox > Mailbox;

		shared_ptr< CExecuteProcessScheduledTask > ExecuteTask;

		unique_ptr< CPlatformThread > PlatformThread;

		ExecuteProcessDelegateType ExecuteDelegate;

		EInternalProcessState State;

		std::set< EProcessID::Enum > PendingShutdownIDs;
};

/**********************************************************************************************************************
	CProcessRecord::CProcessRecord -- constructor

		process -- pointer to the process this record tracks
		execute_delegate -- the function to invoke when it's time to execute the process through a TBB task
					
**********************************************************************************************************************/
CProcessRecord::CProcessRecord( const shared_ptr< IManagedProcess > &process, const ExecuteProcessDelegateType &execute_delegate ) :
	ProcessID( process->Get_ID() ),
	Process( process ),
	Mailbox( new CProcessMailbox( process->Get_ID(), process->Get_Properties() ) ),
	ExecuteTask( nullptr ),
	PlatformThread( nullptr ),
	ExecuteDelegate( execute_delegate ),
	State( EIPS_INITIALIZING ),
	PendingShutdownIDs()
{
	if ( process->Get_Execution_Mode() == EProcessExecutionMode::THREAD )
	{
		PlatformThread.reset( new CPlatformThread );
	}
}

/**********************************************************************************************************************
	CProcessRecord::CProcessRecord -- alternative dummy constructor, only used to proxy the manager

		key -- key of the proxy process
					
**********************************************************************************************************************/
CProcessRecord::CProcessRecord( EProcessID::Enum process_id ) :
	ProcessID( process_id ),
	Process( nullptr ),
	Mailbox( new CProcessMailbox( process_id, MANAGER_PROCESS_PROPERTIES ) ),
	ExecuteTask( nullptr ),
	ExecuteDelegate(),
	State( EIPS_INITIALIZING ),
	PendingShutdownIDs()
{
	FATAL_ASSERT( process_id == EProcessID::CONCURRENCY_MANAGER );
}

/**********************************************************************************************************************
	CProcessRecord::~CProcessRecord -- destructor
					
**********************************************************************************************************************/
CProcessRecord::~CProcessRecord()
{
	FATAL_ASSERT( ExecuteTask == nullptr || !ExecuteTask->Is_Scheduled() );
}

/**********************************************************************************************************************
	CProcessRecord::Add_Execute_Task -- Schedules an execution task for this process at a requested time

		task_scheduler -- task scheduler to schedule the task in
		execution_time -- time the process should be serviced
					
**********************************************************************************************************************/
void CProcessRecord::Add_Execute_Task( const shared_ptr< CTaskScheduler > &task_scheduler, double execution_time )
{
	if ( ExecuteTask == nullptr )
	{
		ExecuteTask.reset( new CExecuteProcessScheduledTask( ExecuteDelegate, Get_Process_ID(), execution_time ) );
	}
	else
	{
		FATAL_ASSERT( !ExecuteTask->Is_Scheduled() );

		ExecuteTask->Set_Execute_Time( execution_time );
	}

	task_scheduler->Submit_Task( ExecuteTask );
}

/**********************************************************************************************************************
	CProcessRecord::Remove_Execute_Task -- removes a process's execution task from a task scheduler

		task_scheduler -- task scheduler to remove the task from
					
**********************************************************************************************************************/
void CProcessRecord::Remove_Execute_Task( const shared_ptr< CTaskScheduler > &task_scheduler )
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
	IDToPropertiesTable(),
	PropertiesToIDTable(),
	PersistentGetRequests(),
	MessageHandlers(),
	PendingOutboundFrames(),
	TaskSchedulers(),
	TimeKeeper( new CTimeKeeper ),
	TBBTaskSchedulerInit( new tbb::task_scheduler_init ),
	State( ECMS_PRE_INITIALIZE ),
	NextID( EProcessID::FIRST_FREE_ID )
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

	CProcessStatics::Set_Concurrency_Manager( this );
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
	PersistentGetRequests.clear();

	FATAL_ASSERT( ProcessRecords.size() == 0 );

	CLogInterface::Shutdown_Dynamic();
	CProcessStatics::Set_Concurrency_Manager( nullptr );

	State = ECMS_FINISHED;
}

/**********************************************************************************************************************
	CConcurrencyManager::Get_Record -- gets the process record for a given process by id

		process_id -- id of the process to get a record for

		Returns: pointer to the process record, or null
					
**********************************************************************************************************************/
shared_ptr< CProcessRecord > CConcurrencyManager::Get_Record( EProcessID::Enum process_id ) const
{
	auto iter = ProcessRecords.find( process_id );
	if ( iter != ProcessRecords.end() )
	{
		return iter->second;
	}

	return nullptr;
}

/**********************************************************************************************************************
	CConcurrencyManager::Get_Process -- gets the process for a given process id

		process_id -- id of the process to look up

		Returns: pointer to the process, or null
					
**********************************************************************************************************************/
shared_ptr< IManagedProcess > CConcurrencyManager::Get_Process( EProcessID::Enum process_id ) const
{
	shared_ptr< CProcessRecord > record = Get_Record( process_id );
	if ( record != nullptr )
	{
		return record->Get_Process();
	}

	return shared_ptr< IManagedProcess >( nullptr );
}

/**********************************************************************************************************************
	CConcurrencyManager::Enumerate_Virtual_Processes -- enumerates all active processes in the system; only used by
		unit tests

		processes -- output parameter for set of all processes
					
**********************************************************************************************************************/
void CConcurrencyManager::Enumerate_Processes( std::vector< shared_ptr< IManagedProcess > > &processes ) const
{
	processes.clear();

	for ( auto iter = ProcessRecords.cbegin(); iter != ProcessRecords.cend(); ++iter )
	{
		if ( iter->first != EProcessID::CONCURRENCY_MANAGER )
		{
			processes.push_back( iter->second->Get_Process() );
		}
	}
}

/**********************************************************************************************************************
	CConcurrencyManager::Run -- starts the concurrency manager system

		starting_thread -- initial thread task
					
**********************************************************************************************************************/
void CConcurrencyManager::Run( const shared_ptr< IManagedProcess > &starting_process )
{
	Setup_For_Run( starting_process );

	Service();
}

/**********************************************************************************************************************
	CConcurrencyManager::Setup_For_Run -- prepares the manager to begin its service loop

		starting_process -- initial virtual process
					
**********************************************************************************************************************/
void CConcurrencyManager::Setup_For_Run( const shared_ptr< IManagedProcess > &starting_process )
{
	FATAL_ASSERT( State == ECMS_INITIALIZED );
	FATAL_ASSERT( ProcessRecords.size() == 0 );

	// make a proxy for the manager
	ProcessRecords[ EProcessID::CONCURRENCY_MANAGER ] = shared_ptr< CProcessRecord >( new CProcessRecord( EProcessID::CONCURRENCY_MANAGER ) );

	// Setup the logging thread and the initial thread
	Add_Process( CLogInterface::Get_Logging_Process(), EProcessID::LOGGING );
	Add_Process( shared_ptr< IManagedProcess>( starting_process ) );

	// Reset time
	TimeKeeper->Set_Base_Time( TT_REAL_TIME, STickTime( CPlatformTime::Get_High_Resolution_Time() ) );
	TimeKeeper->Set_Base_Time( TT_GAME_TIME, STickTime( 0 ) );

	State = ECMS_RUNNING;
}

/**********************************************************************************************************************
	CConcurrencyManager::Add_Process -- adds a new process into the concurrency system

		thread -- process to add to the concurrency system
					
**********************************************************************************************************************/
void CConcurrencyManager::Add_Process( const shared_ptr< IManagedProcess > &process )
{
	Add_Process( process, Allocate_Process_ID() );
}

/**********************************************************************************************************************
	CConcurrencyManager::Add_Process -- adds a new process into the concurrency system

		thread -- process to add to the concurrency system
		id -- id to bind to the process
					
**********************************************************************************************************************/
void CConcurrencyManager::Add_Process( const shared_ptr< IManagedProcess > &process, EProcessID::Enum id )
{
	FATAL_ASSERT( id != EProcessID::CONCURRENCY_MANAGER );
	FATAL_ASSERT( ProcessRecords.find( id ) == ProcessRecords.end() );
	FATAL_ASSERT( process->Get_Properties().Is_Valid() );

	process->Initialize( id );
	if ( id != EProcessID::LOGGING )
	{
		process->Set_Logging_Mailbox( Get_Mailbox( EProcessID::LOGGING ) );
	}

	// track the thread task in a task record
	shared_ptr< CProcessRecord > process_record( new CProcessRecord( process, ExecuteProcessDelegateType( this, &CConcurrencyManager::Execute_Process ) ) );
	ProcessRecords[ id ] = process_record;

	// Interface setup
	CProcessMailbox *mailbox = process_record->Get_Mailbox();
	Handle_Ongoing_Mailbox_Requests( mailbox );

	process->Set_Manager_Mailbox( Get_Mailbox( EProcessID::CONCURRENCY_MANAGER ) );
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
void CConcurrencyManager::Handle_Ongoing_Mailbox_Requests( CProcessMailbox *mailbox )
{
	EProcessID::Enum new_id = mailbox->Get_Process_ID();
	const SProcessProperties &new_properties = mailbox->Get_Properties();

	FATAL_ASSERT( !Is_Process_Shutting_Down( new_id ) );

	// persistent get requests
	for ( auto iter = PersistentGetRequests.cbegin(); iter != PersistentGetRequests.cend(); ++iter )
	{
		EProcessID::Enum requesting_process_id = iter->first;
		const shared_ptr< const CGetMailboxByPropertiesRequest > &get_request = iter->second;
		if ( get_request->Get_Target_Properties().Matches( new_properties ) && requesting_process_id != new_id && !Is_Process_Shutting_Down( requesting_process_id ) )
		{
			shared_ptr< const IProcessMessage > message( new CAddMailboxMessage( Get_Mailbox( new_id ) ) );
			Send_Process_Message( requesting_process_id, message );
		}
	}
}

/**********************************************************************************************************************
	CConcurrencyManager::Get_Mailbox -- gets the write-only mailbox to a process

		process_id -- id of the process to get the mailbox for

		Returns: a pointer to the write-only mailbox, or null
					
**********************************************************************************************************************/
shared_ptr< CWriteOnlyMailbox > CConcurrencyManager::Get_Mailbox( EProcessID::Enum process_id ) const
{
	auto iter = ProcessRecords.find( process_id );
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
	auto iter = ProcessRecords.find( EProcessID::CONCURRENCY_MANAGER );
	FATAL_ASSERT( iter != ProcessRecords.end() );

	return iter->second->Get_Mailbox()->Get_Readable_Mailbox();
}

/**********************************************************************************************************************
	CConcurrencyManager::Send_Process_Message -- queues up a message to a process

		dest_process_id -- the message's destination process
		message -- message to send
					
**********************************************************************************************************************/
void CConcurrencyManager::Send_Process_Message( EProcessID::Enum dest_process_id, const shared_ptr< const IProcessMessage > &message )
{
	auto iter = PendingOutboundFrames.find( dest_process_id );
	if ( iter == PendingOutboundFrames.end() )
	{
		shared_ptr< CProcessMessageFrame > frame( new CProcessMessageFrame( EProcessID::CONCURRENCY_MANAGER ) );
		frame->Add_Message( message );
		PendingOutboundFrames.insert( FrameTableType::value_type( dest_process_id, frame ) );
		return;
	}

	iter->second->Add_Message( message );
}

/**********************************************************************************************************************
	CConcurrencyManager::Flush_Frames -- sends all outbound message frames to their respective process destinations
					
**********************************************************************************************************************/
void CConcurrencyManager::Flush_Frames( void )
{
	std::vector< EProcessID::Enum > sent_frames;

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

	// with the update from key to id, we should never have leftover frames
	FATAL_ASSERT( PendingOutboundFrames.size() == 0 );
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
		FATAL_ASSERT( Get_Record( EProcessID::CONCURRENCY_MANAGER ) != nullptr && Get_Record( EProcessID::LOGGING ) != nullptr );

		State = ECMS_SHUTTING_DOWN_PHASE2;

		shared_ptr< const IProcessMessage > message( new CShutdownSelfRequest( true ) );
		Send_Process_Message( EProcessID::LOGGING, message );
	}
	else if ( ProcessRecords.size() == 1 )
	{
		// Log thread now gone, we're ready to quit
		FATAL_ASSERT( Get_Record( EProcessID::CONCURRENCY_MANAGER ) != nullptr );

		ProcessRecords.clear();
	}
}

/**********************************************************************************************************************
	CConcurrencyManager::Service_Incoming_Frames -- iterates all incoming message frames and handles the messages
		within them
					
**********************************************************************************************************************/
void CConcurrencyManager::Service_Incoming_Frames( void )
{
	std::vector< shared_ptr< CProcessMessageFrame > > control_frames;
	Get_My_Mailbox()->Remove_Frames( control_frames );

	// iterate all frames
	for ( uint32 i = 0; i < control_frames.size(); ++i )
	{
		const shared_ptr< CProcessMessageFrame > &frame = control_frames[ i ];
		EProcessID::Enum source_process_id = frame->Get_Process_ID();

		// iterate all messages in the frame
		for ( auto iter = frame->Get_Frame_Begin(); iter != frame->Get_Frame_End(); ++iter )
		{
			Handle_Message( source_process_id, *iter );
		}
	}
}

/**********************************************************************************************************************
	CConcurrencyManager::Handle_Message -- top-level dispatch function for handling an incoming process message

		source_process_id -- process id of the message sender
		message -- message that was sent to the manager
					
**********************************************************************************************************************/
void CConcurrencyManager::Handle_Message( EProcessID::Enum source_process_id, const shared_ptr< const IProcessMessage > &message )
{
	const IProcessMessage *msg_base = message.get();

	Loki::TypeInfo hash_key( typeid( *msg_base ) );
	auto iter = MessageHandlers.find( hash_key );
	FATAL_ASSERT( iter != MessageHandlers.end() );

	iter->second->Handle_Message( source_process_id, message );
}

/**********************************************************************************************************************
	CConcurrencyManager::Register_Message_Handlers -- registers all message handlers for the concurrency manager
					
**********************************************************************************************************************/
void CConcurrencyManager::Register_Message_Handlers( void )
{
	REGISTER_THIS_HANDLER( CGetMailboxByIDRequest, CConcurrencyManager, Handle_Get_Mailbox_By_ID_Request )
	REGISTER_THIS_HANDLER( CGetMailboxByPropertiesRequest, CConcurrencyManager, Handle_Get_Mailbox_By_Properties_Request )
	REGISTER_THIS_HANDLER( CAddNewProcessMessage, CConcurrencyManager, Handle_Add_New_Process_Message )
	REGISTER_THIS_HANDLER( CShutdownProcessMessage, CConcurrencyManager, Handle_Shutdown_Process_Message )
	REGISTER_THIS_HANDLER( CRescheduleProcessMessage, CConcurrencyManager, Handle_Reschedule_Process_Message )
	REGISTER_THIS_HANDLER( CReleaseMailboxResponse, CConcurrencyManager, Handle_Release_Mailbox_Response )
	REGISTER_THIS_HANDLER( CShutdownSelfResponse, CConcurrencyManager, Handle_Shutdown_Self_Response )
	REGISTER_THIS_HANDLER( CShutdownManagerMessage, CConcurrencyManager, Handle_Shutdown_Manager_Message )
} 

/**********************************************************************************************************************
	CConcurrencyManager::Register_Handler -- registers a single message handler for a message type

		message_type_info -- C++ type info for the message class
		handler -- message handling delegate to invoke for this message type
					
**********************************************************************************************************************/
void CConcurrencyManager::Register_Handler( const std::type_info &message_type_info, const shared_ptr< IProcessMessageHandler > &handler )
{
	Loki::TypeInfo key( message_type_info );

	FATAL_ASSERT( MessageHandlers.find( key ) == MessageHandlers.end() );
	MessageHandlers[ key ] = handler;
}

/**********************************************************************************************************************
	CConcurrencyManager::Handle_Get_Mailbox_By_ID_Request -- message handler for a GetMailboxByID request

		source_process_id -- process source of the request
		message -- the get mailbox request
					
**********************************************************************************************************************/
void CConcurrencyManager::Handle_Get_Mailbox_By_ID_Request( EProcessID::Enum source_process_id, const shared_ptr< const CGetMailboxByIDRequest > &message )
{
	// don't handle messages while shutting down
	if ( Is_Manager_Shutting_Down() )
	{
		return;
	}

	// don't give out interfaces when a thread is shutting down
	EProcessID::Enum requested_process_id = message->Get_Target_Process_ID();
	if ( Is_Process_Shutting_Down( source_process_id ) )
	{
		return;
	}

	// it's a single-targeted request
	shared_ptr< CProcessRecord > record = Get_Record( requested_process_id );
	if ( record != nullptr && !record->Is_Shutting_Down() && source_process_id != requested_process_id )
	{
		// fulfill the request
		shared_ptr< const IProcessMessage > message( new CAddMailboxMessage( record->Get_Mailbox()->Get_Writable_Mailbox() ) );
		Send_Process_Message( source_process_id, message );
	}
}

/**********************************************************************************************************************
	CConcurrencyManager::Handle_Get_Mailbox_By_Properties_Request -- message handler for a GetMailboxByProperties request

		source_process_id -- process source of the request
		message -- the get mailbox request
					
**********************************************************************************************************************/
void CConcurrencyManager::Handle_Get_Mailbox_By_Properties_Request( EProcessID::Enum source_process_id, const shared_ptr< const CGetMailboxByPropertiesRequest > &message )
{
	// don't handle messages while shutting down
	if ( Is_Manager_Shutting_Down() )
	{
		return;
	}

	// don't give out interfaces when a thread is shutting down
	const SProcessProperties &requested_properties = message->Get_Target_Properties();
	if ( Is_Process_Shutting_Down( source_process_id ) )
	{
		return;
	}

	// it's a persistent pattern-matching request, match all existing threads and track against future adds
	for ( auto iter = ProcessRecords.cbegin(); iter != ProcessRecords.cend(); ++iter )
	{
		if ( requested_properties.Matches( iter->second->Get_Properties() ) && !Is_Process_Shutting_Down( iter->first ) && source_process_id != iter->first )
		{
			shared_ptr< const IProcessMessage > message( new CAddMailboxMessage( Get_Mailbox( iter->first ) ) );
			Send_Process_Message( source_process_id, message );
		}
	}

	PersistentGetRequests.insert( GetMailboxByPropertiesRequestCollectionType::value_type( source_process_id, message ) );
}

/**********************************************************************************************************************
	CConcurrencyManager::Handle_Add_New_Process_Message -- message handler for an AddNewProcess message

		source_process_id -- process source of the request
		message -- the add process message
					
**********************************************************************************************************************/
void CConcurrencyManager::Handle_Add_New_Process_Message( EProcessID::Enum source_process_id, const shared_ptr< const CAddNewProcessMessage > &message )
{
	if ( Is_Manager_Shutting_Down() )
	{
		return;
	}

	Add_Process( static_pointer_cast< IManagedProcess >( message->Get_Process() ) );

	// return mailbox, push mailbox options
	if ( !Is_Process_Shutting_Down( source_process_id ) )
	{
		if ( message->Should_Return_Mailbox() )
		{
			shared_ptr< const IProcessMessage > response_message( new CAddMailboxMessage( Get_Mailbox( message->Get_Process()->Get_ID() ) ) );
			Send_Process_Message( source_process_id, response_message );
		}

		if ( message->Should_Forward_Creator_Mailbox() )
		{
			shared_ptr< const IProcessMessage > response_message( new CAddMailboxMessage( Get_Mailbox( source_process_id ) ) );
			Send_Process_Message( message->Get_Process()->Get_ID(), response_message );
		}
	}
}

/**********************************************************************************************************************
	CConcurrencyManager::Handle_Shutdown_Process_Message -- message handler for an ShutdownVirtualProcess message

		source_process_id -- process source of the message
		message -- the shutdown process message
					
**********************************************************************************************************************/
void CConcurrencyManager::Handle_Shutdown_Process_Message( EProcessID::Enum /*source_process_id*/, const shared_ptr< const CShutdownProcessMessage > &message )
{
	if ( Is_Manager_Shutting_Down() )
	{
		return;
	}

	Initiate_Process_Shutdown( message->Get_Process_ID() );
}

/**********************************************************************************************************************
	CConcurrencyManager::Handle_Reschedule_Process_Message -- message handler for a RescheduleProcess message

		source_process_id -- process source of the message
		message -- the reschedule process message
					
**********************************************************************************************************************/
void CConcurrencyManager::Handle_Reschedule_Process_Message( EProcessID::Enum source_process_id, const shared_ptr< const CRescheduleProcessMessage > &message )
{
	shared_ptr< CProcessRecord > record = Get_Record( source_process_id );
	if ( record == nullptr )
	{
		return;
	}

	ETimeType time_type = record->Get_Process()->Get_Time_Type();
	double execute_time = std::min( message->Get_Reschedule_Time(), TimeKeeper->Get_Elapsed_Seconds( time_type ) );
	record->Add_Execute_Task( Get_Task_Scheduler( time_type ), execute_time );
}

/**********************************************************************************************************************
	CConcurrencyManager::Handle_Release_Mailbox_Response -- message handler for a ReleaseMailbox response

		source_process_id -- process source of the response
		response -- the release mailbox response
					
**********************************************************************************************************************/
void CConcurrencyManager::Handle_Release_Mailbox_Response( EProcessID::Enum source_process_id, const shared_ptr< const CReleaseMailboxResponse > &response )
{
	if ( Is_Manager_Shutting_Down() )
	{
		return;
	}

	EProcessID::Enum shutdown_process_id = response->Get_Shutdown_Process_ID();
	shared_ptr< CProcessRecord > record = Get_Record( shutdown_process_id );
	if ( record == nullptr )
	{
		return;
	}

	FATAL_ASSERT( record->Get_State() == EIPS_SHUTTING_DOWN_PHASE1 );

	record->Remove_Pending_Shutdown_PID( source_process_id );
	if ( !record->Has_Pending_Shutdown_IDs() )
	{
		// we've heard back from everyone; no one has a handle to this thread anymore, so we can tell it to shut down
		record->Set_State( EIPS_SHUTTING_DOWN_PHASE2 );

		shared_ptr< const IProcessMessage > shutdown_request( new CShutdownSelfRequest( false ) );
		Send_Process_Message( shutdown_process_id, shutdown_request );
	}
}

/**********************************************************************************************************************
	CConcurrencyManager::Handle_Shutdown_Self_Response -- message handler for a ShutdownSelf response

		source_process_id -- process source of the response
		message -- the shutdown self response
					
**********************************************************************************************************************/
void CConcurrencyManager::Handle_Shutdown_Self_Response( EProcessID::Enum source_process_id, const shared_ptr< const CShutdownSelfResponse > & /*message*/ )
{
	auto iter = ProcessRecords.find( source_process_id );
	FATAL_ASSERT( iter != ProcessRecords.end() );
	FATAL_ASSERT( iter->second->Get_State() == EIPS_SHUTTING_DOWN_PHASE2 || Is_Manager_Shutting_Down() );

	ProcessRecords.erase( iter );
}

/**********************************************************************************************************************
	CConcurrencyManager::Handle_Shutdown_Manager_Message -- message handler for a ShutdownManager message

		source_process_id -- process source of the message
		message -- the shutdown manager message
					
**********************************************************************************************************************/
void CConcurrencyManager::Handle_Shutdown_Manager_Message( EProcessID::Enum /*source_process_id*/, const shared_ptr< const CShutdownManagerMessage > & /*message*/ )
{
	if ( Is_Manager_Shutting_Down() )
	{
		return;
	}

	State = ECMS_SHUTTING_DOWN_PHASE1;

	// tell everyone to shut down
	shared_ptr< const IProcessMessage > shutdown_thread_msg( new CShutdownSelfRequest( true ) );
	for ( auto iter = ProcessRecords.cbegin(); iter != ProcessRecords.cend(); ++iter )
	{
		if ( iter->first == EProcessID::CONCURRENCY_MANAGER || iter->first == EProcessID::LOGGING )
		{
			continue;
		}

		Send_Process_Message( iter->first, shutdown_thread_msg );
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
	CConcurrencyManager::Execute_Process -- queues up a tbb task to execute the supplied process's service

		process_id -- id of the process to have executed
		current_time_seconds -- current time from the process's standpoint
					
**********************************************************************************************************************/
void CConcurrencyManager::Execute_Process( EProcessID::Enum process_id, double current_time_seconds )
{
	auto iter = ProcessRecords.find( process_id );
	if ( iter == ProcessRecords.end() )
	{
		return;
	}

	shared_ptr< CProcessRecord > record = iter->second;
	if ( record->Get_State() == EIPS_INITIALIZING )
	{
		record->Set_State( EIPS_RUNNING );
	}

	shared_ptr< IManagedProcess > thread_task_base = record->Get_Process();

	switch ( thread_task_base->Get_Execution_Mode() )
	{
		case EProcessExecutionMode::TASK:
			if ( process_id == EProcessID::LOGGING )
			{
				CServiceLoggingProcessTBBTask &tbb_task = *new( tbb::task::allocate_root() ) CServiceLoggingProcessTBBTask( current_time_seconds );
				tbb::task::enqueue( tbb_task );
			}
			else
			{
				CServiceProcessTBBTask &tbb_task = *new( tbb::task::allocate_root() ) CServiceProcessTBBTask( thread_task_base, current_time_seconds );
				tbb::task::enqueue( tbb_task );
			}

			break;

		case EProcessExecutionMode::THREAD:
		{
			CProcessExecutionContext context( record->Get_Platform_Thread() );
			thread_task_base->Run( context );

			break;
		}

		default:
			FATAL_ASSERT( false );
			break;
	}
}

/**********************************************************************************************************************
	CConcurrencyManager::Initiate_Process_Shutdown -- guides a process into the shutdown procedire

		process_id -- ID of the process to start shutting down
					
**********************************************************************************************************************/
void CConcurrencyManager::Initiate_Process_Shutdown( EProcessID::Enum process_id )
{
	FATAL_ASSERT( process_id != EProcessID::CONCURRENCY_MANAGER && process_id != EProcessID::LOGGING );

	shared_ptr< CProcessRecord > shutdown_record = Get_Record( process_id );
	if ( shutdown_record == nullptr || shutdown_record->Is_Shutting_Down() )
	{
		return;
	}

	shutdown_record->Set_State( EIPS_SHUTTING_DOWN_PHASE1 );

	// this message can be shared and broadcast
	shared_ptr< const IProcessMessage > release_mailbox_msg( new CReleaseMailboxRequest( process_id ) );
	for ( auto iter = ProcessRecords.cbegin(); iter != ProcessRecords.cend(); ++iter )
	{
		if ( process_id == iter->first || iter->first == EProcessID::CONCURRENCY_MANAGER || iter->first == EProcessID::LOGGING )
		{
			continue;
		}

		shutdown_record->Add_Pending_Shutdown_PID( iter->first );
		Send_Process_Message( iter->first, release_mailbox_msg );
	}

	// Remove all push/get requests related to this process
	Clear_Related_Mailbox_Requests( process_id );

	// Move to the next state if we're not waiting on any other process acknowledgements
	if ( !shutdown_record->Has_Pending_Shutdown_IDs() )
	{
		shutdown_record->Set_State( EIPS_SHUTTING_DOWN_PHASE2 );

		shared_ptr< const IProcessMessage > shutdown_request( new CShutdownSelfRequest( false ) );
		Send_Process_Message( process_id, shutdown_request );
	}
}

/**********************************************************************************************************************
	CConcurrencyManager::Clear_Related_Mailbox_Requests -- clears mailbox requests related to a thread

		process_id -- id of the virtual process to clear mailbox requests about
					
**********************************************************************************************************************/
void CConcurrencyManager::Clear_Related_Mailbox_Requests( EProcessID::Enum process_id )
{
	PersistentGetRequests.erase( PersistentGetRequests.lower_bound( process_id ), PersistentGetRequests.upper_bound( process_id ) );
}

/**********************************************************************************************************************
	CConcurrencyManager::Is_Process_Shutting_Down -- asks if the supplied process is in the process of shutting down

		process_id -- id of the process to check shutdown status of

		Returns: true if the process is shutting down, false otherwise
					
**********************************************************************************************************************/
bool CConcurrencyManager::Is_Process_Shutting_Down( EProcessID::Enum process_id ) const
{
	shared_ptr< CProcessRecord > record = Get_Record( process_id );
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
		Send_Process_Message( EProcessID::LOGGING, shared_ptr< const IProcessMessage >( new CLogRequestMessage( MANAGER_PROCESS_PROPERTIES, message ) ) );
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

/**********************************************************************************************************************
	CConcurrencyManager::Allocate_Virtual_Process_ID -- allocates a new process ID

		Returns: a new process id
					
**********************************************************************************************************************/
EProcessID::Enum CConcurrencyManager::Allocate_Process_ID( void )
{
	EProcessID::Enum id = NextID;
	NextID = static_cast< EProcessID::Enum >( NextID + 1 );

	return id;
}
