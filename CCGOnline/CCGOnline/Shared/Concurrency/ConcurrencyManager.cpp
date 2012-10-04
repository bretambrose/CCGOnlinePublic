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
#include "VirtualProcessSubject.h"
#include "VirtualProcessMailbox.h"
#include "MailboxInterfaces.h"
#include "VirtualProcessConstants.h"
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
#include "VirtualProcessID.h"


typedef FastDelegate2< EVirtualProcessID::Enum, double, void > ExecuteProcessDelegateType;

// A scheduled task that triggers the execution of a scheduled process's service function by TBB
class CExecuteProcessScheduledTask : public CScheduledTask
{
	public:

		typedef CScheduledTask BASECLASS;

		CExecuteProcessScheduledTask( const ExecuteProcessDelegateType &execute_delegate, EVirtualProcessID::Enum process_id, double execute_time_seconds ) :
			BASECLASS( execute_time_seconds ),
			ExecuteDelegate( execute_delegate ),
			ProcessID( process_id )
		{}

		EVirtualProcessID::Enum Get_Process_ID( void ) const { return ProcessID; }

		virtual bool Execute( double current_time_seconds, double & /*reschedule_time_seconds*/ )
		{
			ExecuteDelegate( ProcessID, current_time_seconds );

			return false;
		}

	private:

		ExecuteProcessDelegateType ExecuteDelegate;
		EVirtualProcessID::Enum ProcessID;
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
		CVirtualProcessRecord( EVirtualProcessID::Enum process_id );
		~CVirtualProcessRecord();

		// Accessors
		EVirtualProcessID::Enum Get_Process_ID( void ) const { return ProcessID; }
		SProcessProperties Get_Properties( void ) const {
			if ( ProcessID == EVirtualProcessID::CONCURRENCY_MANAGER )
			{
				return MANAGER_PROCESS_PROPERTIES;
			}
			else
			{
				return Process->Get_Properties();
			}
		}

		shared_ptr< IManagedVirtualProcess > Get_Virtual_Process( void ) const { return Process; }

		EProcessState Get_State( void ) const { return State; }
		void Set_State( EProcessState state ) { State = state; }

		CVirtualProcessMailbox *Get_Mailbox( void ) const { return Mailbox.get(); }

		// Operations
		void Add_Execute_Task( const shared_ptr< CTaskScheduler > &task_scheduler, double execution_time );
		void Remove_Execute_Task( const shared_ptr< CTaskScheduler > &task_scheduler );

		void Add_Pending_Shutdown_PID( EVirtualProcessID::Enum process_id ) { PendingShutdownIDs.insert( process_id ); }
		void Remove_Pending_Shutdown_PID( EVirtualProcessID::Enum process_id ) { PendingShutdownIDs.erase( process_id ); }

		// Queries
		bool Is_Shutting_Down( void ) const { return State == EPS_SHUTTING_DOWN_PHASE1 || State == EPS_SHUTTING_DOWN_PHASE2; }

		bool Has_Pending_Shutdown_IDs( void ) const { return PendingShutdownIDs.size() > 0; }

	private:

		EVirtualProcessID::Enum ProcessID;

		shared_ptr< IManagedVirtualProcess > Process;

		scoped_ptr< CVirtualProcessMailbox > Mailbox;

		shared_ptr< CExecuteProcessScheduledTask > ExecuteTask;

		ExecuteProcessDelegateType ExecuteDelegate;

		EProcessState State;

		std::set< EVirtualProcessID::Enum > PendingShutdownIDs;
};

/**********************************************************************************************************************
	CVirtualProcessRecord::CVirtualProcessRecord -- constructor

		process -- pointer to the process this record tracks
		execute_delegate -- the function to invoke when it's time to execute the process through a TBB task
					
**********************************************************************************************************************/
CVirtualProcessRecord::CVirtualProcessRecord( const shared_ptr< IManagedVirtualProcess > &process, const ExecuteProcessDelegateType &execute_delegate ) :
	ProcessID( process->Get_ID() ),
	Process( process ),
	Mailbox( new CVirtualProcessMailbox( process->Get_ID(), process->Get_Properties() ) ),
	ExecuteTask( nullptr ),
	ExecuteDelegate( execute_delegate ),
	State( EPS_INITIALIZING ),
	PendingShutdownIDs()
{
}

/**********************************************************************************************************************
	CVirtualProcessRecord::CVirtualProcessRecord -- alternative dummy constructor, only used to proxy the manager

		key -- key of the proxy process
					
**********************************************************************************************************************/
CVirtualProcessRecord::CVirtualProcessRecord( EVirtualProcessID::Enum process_id ) :
	ProcessID( process_id ),
	Process( nullptr ),
	Mailbox( new CVirtualProcessMailbox( process_id, MANAGER_PROCESS_PROPERTIES ) ),
	ExecuteTask( nullptr ),
	ExecuteDelegate(),
	State( EPS_INITIALIZING ),
	PendingShutdownIDs()
{
	FATAL_ASSERT( process_id == EVirtualProcessID::CONCURRENCY_MANAGER );
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
	IDToPropertiesTable(),
	PropertiesToIDTable(),
	PersistentGetRequests(),
	MessageHandlers(),
	PendingOutboundFrames(),
	TaskSchedulers(),
	TimeKeeper( new CTimeKeeper ),
	TBBTaskSchedulerInit( new tbb::task_scheduler_init ),
	State( ECMS_PRE_INITIALIZE ),
	NextID( EVirtualProcessID::FIRST_FREE_ID )
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
	PersistentGetRequests.clear();

	FATAL_ASSERT( ProcessRecords.size() == 0 );

	CLogInterface::Shutdown_Dynamic();
	CVirtualProcessStatics::Set_Concurrency_Manager( nullptr );

	State = ECMS_FINISHED;
}

/**********************************************************************************************************************
	CConcurrencyManager::Get_Record -- gets the process record for a given process by id

		process_id -- id of the process to get a record for

		Returns: pointer to the process record, or null
					
**********************************************************************************************************************/
shared_ptr< CVirtualProcessRecord > CConcurrencyManager::Get_Record( EVirtualProcessID::Enum process_id ) const
{
	auto iter = ProcessRecords.find( process_id );
	if ( iter != ProcessRecords.end() )
	{
		return iter->second;
	}

	return nullptr;
}

/**********************************************************************************************************************
	CConcurrencyManager::Get_Virtual_Process -- gets the process for a given process id

		process_id -- id of the process to look up

		Returns: pointer to the virtual process, or null
					
**********************************************************************************************************************/
shared_ptr< IManagedVirtualProcess > CConcurrencyManager::Get_Virtual_Process( EVirtualProcessID::Enum process_id ) const
{
	shared_ptr< CVirtualProcessRecord > record = Get_Record( process_id );
	if ( record != nullptr )
	{
		return record->Get_Virtual_Process();
	}

	return shared_ptr< IManagedVirtualProcess >( nullptr );
}

/**********************************************************************************************************************
	CConcurrencyManager::Enumerate_Virtual_Processes -- enumerates all active processes in the system; only used by
		unit tests

		processes -- output parameter for set of all processes
					
**********************************************************************************************************************/
void CConcurrencyManager::Enumerate_Virtual_Processes( std::vector< shared_ptr< IManagedVirtualProcess > > &processes ) const
{
	processes.clear();

	for ( auto iter = ProcessRecords.cbegin(); iter != ProcessRecords.cend(); ++iter )
	{
		if ( iter->first != EVirtualProcessID::CONCURRENCY_MANAGER )
		{
			processes.push_back( iter->second->Get_Virtual_Process() );
		}
	}
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
	ProcessRecords[ EVirtualProcessID::CONCURRENCY_MANAGER ] = shared_ptr< CVirtualProcessRecord >( new CVirtualProcessRecord( EVirtualProcessID::CONCURRENCY_MANAGER ) );

	// Setup the logging thread and the initial thread
	Add_Virtual_Process( CLogInterface::Get_Logging_Process(), EVirtualProcessID::LOGGING );
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
	Add_Virtual_Process( process, Allocate_Virtual_Process_ID() );
}

/**********************************************************************************************************************
	CConcurrencyManager::Add_Virtual_Process -- adds a new virtual process into the concurrency system

		thread -- virtual process to add to the concurrency system
		id -- id to bind to the process
					
**********************************************************************************************************************/
void CConcurrencyManager::Add_Virtual_Process( const shared_ptr< IManagedVirtualProcess > &process, EVirtualProcessID::Enum id )
{
	FATAL_ASSERT( id != EVirtualProcessID::CONCURRENCY_MANAGER );
	FATAL_ASSERT( ProcessRecords.find( id ) == ProcessRecords.end() );
	FATAL_ASSERT( process->Get_Properties().Is_Valid() );

	process->Initialize( id );
	if ( id != EVirtualProcessID::LOGGING )
	{
		process->Set_Logging_Mailbox( Get_Mailbox( EVirtualProcessID::LOGGING ) );
	}

	// track the thread task in a task record
	shared_ptr< CVirtualProcessRecord > process_record( new CVirtualProcessRecord( process, ExecuteProcessDelegateType( this, &CConcurrencyManager::Execute_Virtual_Process ) ) );
	ProcessRecords[ id ] = process_record;

	// Interface setup
	CVirtualProcessMailbox *mailbox = process_record->Get_Mailbox();
	Handle_Ongoing_Mailbox_Requests( mailbox );

	process->Set_Manager_Mailbox( Get_Mailbox( EVirtualProcessID::CONCURRENCY_MANAGER ) );
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
	EVirtualProcessID::Enum new_id = mailbox->Get_Process_ID();
	const SProcessProperties &new_properties = mailbox->Get_Properties();

	FATAL_ASSERT( !Is_Process_Shutting_Down( new_id ) );

	// persistent get requests
	for ( auto iter = PersistentGetRequests.cbegin(); iter != PersistentGetRequests.cend(); ++iter )
	{
		EVirtualProcessID::Enum requesting_process_id = iter->first;
		const shared_ptr< const CGetMailboxByPropertiesRequest > &get_request = iter->second;
		if ( get_request->Get_Target_Properties().Matches( new_properties ) && requesting_process_id != new_id && !Is_Process_Shutting_Down( requesting_process_id ) )
		{
			shared_ptr< const IVirtualProcessMessage > message( new CAddMailboxMessage( Get_Mailbox( new_id ) ) );
			Send_Virtual_Process_Message( requesting_process_id, message );
		}
	}
}

/**********************************************************************************************************************
	CConcurrencyManager::Get_Mailbox -- gets the write-only mailbox to a virtual process

		process_id -- id of the virtual process to get the mailbox for

		Returns: a pointer to the write-only mailbox, or null
					
**********************************************************************************************************************/
shared_ptr< CWriteOnlyMailbox > CConcurrencyManager::Get_Mailbox( EVirtualProcessID::Enum process_id ) const
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
	auto iter = ProcessRecords.find( EVirtualProcessID::CONCURRENCY_MANAGER );
	FATAL_ASSERT( iter != ProcessRecords.end() );

	return iter->second->Get_Mailbox()->Get_Readable_Mailbox();
}

/**********************************************************************************************************************
	CConcurrencyManager::Send_Virtual_Process_Message -- queues up a message to a virtual process

		dest_key -- the message's destination process
		message -- message to send
					
**********************************************************************************************************************/
void CConcurrencyManager::Send_Virtual_Process_Message( EVirtualProcessID::Enum dest_process_id, const shared_ptr< const IVirtualProcessMessage > &message )
{
	auto iter = PendingOutboundFrames.find( dest_process_id );
	if ( iter == PendingOutboundFrames.end() )
	{
		shared_ptr< CVirtualProcessMessageFrame > frame( new CVirtualProcessMessageFrame( EVirtualProcessID::CONCURRENCY_MANAGER ) );
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
	std::vector< EVirtualProcessID::Enum > sent_frames;

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
		FATAL_ASSERT( Get_Record( EVirtualProcessID::CONCURRENCY_MANAGER ) != nullptr && Get_Record( EVirtualProcessID::LOGGING ) != nullptr );

		State = ECMS_SHUTTING_DOWN_PHASE2;

		shared_ptr< const IVirtualProcessMessage > message( new CShutdownSelfRequest( true ) );
		Send_Virtual_Process_Message( EVirtualProcessID::LOGGING, message );
	}
	else if ( ProcessRecords.size() == 1 )
	{
		// Log thread now gone, we're ready to quit
		FATAL_ASSERT( Get_Record( EVirtualProcessID::CONCURRENCY_MANAGER ) != nullptr );

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
		EVirtualProcessID::Enum source_process_id = frame->Get_Process_ID();

		// iterate all messages in the frame
		for ( auto iter = frame->Get_Frame_Begin(); iter != frame->Get_Frame_End(); ++iter )
		{
			Handle_Message( source_process_id, *iter );
		}
	}
}

/**********************************************************************************************************************
	CConcurrencyManager::Handle_Message -- top-level dispatch function for handling an incoming thread message

		key -- key of the message sender
		message -- message that was sent to the manager
					
**********************************************************************************************************************/
void CConcurrencyManager::Handle_Message( EVirtualProcessID::Enum source_process_id, const shared_ptr< const IVirtualProcessMessage > &message )
{
	const IVirtualProcessMessage *msg_base = message.get();

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
	CConcurrencyManager::Handle_Get_Mailbox_By_ID_Request -- message handler for a GetMailboxByID request

		source_process_id -- process source of the request
		message -- the get mailbox request
					
**********************************************************************************************************************/
void CConcurrencyManager::Handle_Get_Mailbox_By_ID_Request( EVirtualProcessID::Enum source_process_id, const shared_ptr< const CGetMailboxByIDRequest > &message )
{
	// don't handle messages while shutting down
	if ( Is_Manager_Shutting_Down() )
	{
		return;
	}

	// don't give out interfaces when a thread is shutting down
	EVirtualProcessID::Enum requested_process_id = message->Get_Target_Process_ID();
	if ( Is_Process_Shutting_Down( source_process_id ) )
	{
		return;
	}

	// it's a single-targeted request
	shared_ptr< CVirtualProcessRecord > record = Get_Record( requested_process_id );
	if ( record != nullptr && !record->Is_Shutting_Down() && source_process_id != requested_process_id )
	{
		// fulfill the request
		shared_ptr< const IVirtualProcessMessage > message( new CAddMailboxMessage( record->Get_Mailbox()->Get_Writable_Mailbox() ) );
		Send_Virtual_Process_Message( source_process_id, message );
	}
}

/**********************************************************************************************************************
	CConcurrencyManager::Handle_Get_Mailbox_By_Properties_Request -- message handler for a GetMailboxByProperties request

		source_process_id -- process source of the request
		message -- the get mailbox request
					
**********************************************************************************************************************/
void CConcurrencyManager::Handle_Get_Mailbox_By_Properties_Request( EVirtualProcessID::Enum source_process_id, const shared_ptr< const CGetMailboxByPropertiesRequest > &message )
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
			shared_ptr< const IVirtualProcessMessage > message( new CAddMailboxMessage( Get_Mailbox( iter->first ) ) );
			Send_Virtual_Process_Message( source_process_id, message );
		}
	}

	PersistentGetRequests.insert( GetMailboxByPropertiesRequestCollectionType::value_type( source_process_id, message ) );
}

/**********************************************************************************************************************
	CConcurrencyManager::Handle_Add_New_Virtual_Process_Message -- message handler for an AddNewVirtualProcess message

		source_process_id -- process source of the request
		message -- the add process message
					
**********************************************************************************************************************/
void CConcurrencyManager::Handle_Add_New_Virtual_Process_Message( EVirtualProcessID::Enum source_process_id, const shared_ptr< const CAddNewVirtualProcessMessage > &message )
{
	if ( Is_Manager_Shutting_Down() )
	{
		return;
	}

	Add_Virtual_Process( static_pointer_cast< IManagedVirtualProcess >( message->Get_Virtual_Process() ) );

	// return mailbox, push mailbox options
	if ( !Is_Process_Shutting_Down( source_process_id ) )
	{
		if ( message->Should_Return_Mailbox() )
		{
			shared_ptr< const IVirtualProcessMessage > response_message( new CAddMailboxMessage( Get_Mailbox( message->Get_Virtual_Process()->Get_ID() ) ) );
			Send_Virtual_Process_Message( source_process_id, response_message );
		}

		if ( message->Should_Forward_Creator_Mailbox() )
		{
			shared_ptr< const IVirtualProcessMessage > response_message( new CAddMailboxMessage( Get_Mailbox( source_process_id ) ) );
			Send_Virtual_Process_Message( message->Get_Virtual_Process()->Get_ID(), response_message );
		}
	}
}

/**********************************************************************************************************************
	CConcurrencyManager::Handle_Shutdown_Virtual_Process_Message -- message handler for an ShutdownVirtualProcess message

		source_process_id -- process source of the message
		message -- the shutdown process message
					
**********************************************************************************************************************/
void CConcurrencyManager::Handle_Shutdown_Virtual_Process_Message( EVirtualProcessID::Enum /*source_process_id*/, const shared_ptr< const CShutdownVirtualProcessMessage > &message )
{
	if ( Is_Manager_Shutting_Down() )
	{
		return;
	}

	Initiate_Process_Shutdown( message->Get_Process_ID() );
}

/**********************************************************************************************************************
	CConcurrencyManager::Handle_Reschedule_Virtual_Process_Message -- message handler for a RescheduleVirtualProcess message

		source_process_id -- process source of the message
		message -- the reschedule virtual process message
					
**********************************************************************************************************************/
void CConcurrencyManager::Handle_Reschedule_Virtual_Process_Message( EVirtualProcessID::Enum source_process_id, const shared_ptr< const CRescheduleVirtualProcessMessage > &message )
{
	shared_ptr< CVirtualProcessRecord > record = Get_Record( source_process_id );
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

		source_process_id -- process source of the response
		response -- the release mailbox response
					
**********************************************************************************************************************/
void CConcurrencyManager::Handle_Release_Mailbox_Response( EVirtualProcessID::Enum source_process_id, const shared_ptr< const CReleaseMailboxResponse > &response )
{
	if ( Is_Manager_Shutting_Down() )
	{
		return;
	}

	EVirtualProcessID::Enum shutdown_process_id = response->Get_Shutdown_Process_ID();
	shared_ptr< CVirtualProcessRecord > record = Get_Record( shutdown_process_id );
	if ( record == nullptr )
	{
		return;
	}

	FATAL_ASSERT( record->Get_State() == EPS_SHUTTING_DOWN_PHASE1 );

	record->Remove_Pending_Shutdown_PID( source_process_id );
	if ( !record->Has_Pending_Shutdown_IDs() )
	{
		// we've heard back from everyone; no one has a handle to this thread anymore, so we can tell it to shut down
		record->Set_State( EPS_SHUTTING_DOWN_PHASE2 );

		shared_ptr< const IVirtualProcessMessage > shutdown_request( new CShutdownSelfRequest( false ) );
		Send_Virtual_Process_Message( shutdown_process_id, shutdown_request );
	}
}

/**********************************************************************************************************************
	CConcurrencyManager::Handle_Shutdown_Self_Response -- message handler for a ShutdownSelf response

		source_process_id -- process source of the response
		message -- the shutdown self response
					
**********************************************************************************************************************/
void CConcurrencyManager::Handle_Shutdown_Self_Response( EVirtualProcessID::Enum source_process_id, const shared_ptr< const CShutdownSelfResponse > & /*message*/ )
{
	auto iter = ProcessRecords.find( source_process_id );
	FATAL_ASSERT( iter != ProcessRecords.end() );
	FATAL_ASSERT( iter->second->Get_State() == EPS_SHUTTING_DOWN_PHASE2 || Is_Manager_Shutting_Down() );

	ProcessRecords.erase( iter );
}

/**********************************************************************************************************************
	CConcurrencyManager::Handle_Shutdown_Manager_Message -- message handler for a ShutdownManager message

		key -- thread source of the message
		message -- the shutdown manager message
					
**********************************************************************************************************************/
void CConcurrencyManager::Handle_Shutdown_Manager_Message( EVirtualProcessID::Enum /*source_process_id*/, const shared_ptr< const CShutdownManagerMessage > & /*message*/ )
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
		if ( iter->first == EVirtualProcessID::CONCURRENCY_MANAGER || iter->first == EVirtualProcessID::LOGGING )
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
void CConcurrencyManager::Execute_Virtual_Process( EVirtualProcessID::Enum process_id, double current_time_seconds )
{
	auto iter = ProcessRecords.find( process_id );
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

	if ( process_id == EVirtualProcessID::LOGGING )
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

		process_id -- ID of the process to start shutting down
					
**********************************************************************************************************************/
void CConcurrencyManager::Initiate_Process_Shutdown( EVirtualProcessID::Enum process_id )
{
	FATAL_ASSERT( process_id != EVirtualProcessID::CONCURRENCY_MANAGER && process_id != EVirtualProcessID::LOGGING );

	shared_ptr< CVirtualProcessRecord > shutdown_record = Get_Record( process_id );
	if ( shutdown_record == nullptr || shutdown_record->Is_Shutting_Down() )
	{
		return;
	}

	shutdown_record->Set_State( EPS_SHUTTING_DOWN_PHASE1 );

	// this message can be shared and broadcast
	shared_ptr< const IVirtualProcessMessage > release_mailbox_msg( new CReleaseMailboxRequest( process_id ) );
	for ( auto iter = ProcessRecords.cbegin(); iter != ProcessRecords.cend(); ++iter )
	{
		if ( process_id == iter->first || iter->first == EVirtualProcessID::CONCURRENCY_MANAGER || iter->first == EVirtualProcessID::LOGGING )
		{
			continue;
		}

		shutdown_record->Add_Pending_Shutdown_PID( iter->first );
		Send_Virtual_Process_Message( iter->first, release_mailbox_msg );
	}

	// Remove all push/get requests related to this process
	Clear_Related_Mailbox_Requests( process_id );

	// Move to the next state if we're not waiting on any other process acknowledgements
	if ( !shutdown_record->Has_Pending_Shutdown_IDs() )
	{
		shutdown_record->Set_State( EPS_SHUTTING_DOWN_PHASE2 );

		shared_ptr< const IVirtualProcessMessage > shutdown_request( new CShutdownSelfRequest( false ) );
		Send_Virtual_Process_Message( process_id, shutdown_request );
	}
}

/**********************************************************************************************************************
	CConcurrencyManager::Clear_Related_Mailbox_Requests -- clears mailbox requests related to a thread

		process_id -- id of the virtual process to clear mailbox requests about
					
**********************************************************************************************************************/
void CConcurrencyManager::Clear_Related_Mailbox_Requests( EVirtualProcessID::Enum process_id )
{
	PersistentGetRequests.erase( PersistentGetRequests.lower_bound( process_id ), PersistentGetRequests.upper_bound( process_id ) );
}

/**********************************************************************************************************************
	CConcurrencyManager::Is_Process_Shutting_Down -- asks if the supplied process is in the process of shutting down

		process_id -- id of the virtual process to check shutdown status of

		Returns: true if the process is shutting down, false otherwise
					
**********************************************************************************************************************/
bool CConcurrencyManager::Is_Process_Shutting_Down( EVirtualProcessID::Enum process_id ) const
{
	shared_ptr< CVirtualProcessRecord > record = Get_Record( process_id );
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
		Send_Virtual_Process_Message( EVirtualProcessID::LOGGING, shared_ptr< const IVirtualProcessMessage >( new CLogRequestMessage( MANAGER_PROCESS_PROPERTIES, message ) ) );
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
EVirtualProcessID::Enum CConcurrencyManager::Allocate_Virtual_Process_ID( void )
{
	EVirtualProcessID::Enum id = NextID;
	NextID = static_cast< EVirtualProcessID::Enum >( NextID + 1 );

	return id;
}
