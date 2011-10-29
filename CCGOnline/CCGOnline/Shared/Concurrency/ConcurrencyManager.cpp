/**********************************************************************************************************************

	[Placeholder for eventual source license]

	ConcurrencyManager.cpp
		A component definining the central, manager class that manages all thread tasks

	(c) Copyright 2011, Bret Ambrose.  All rights reserved.

**********************************************************************************************************************/

#include "stdafx.h"

#include "ConcurrencyManager.h"

#include "Logging/LogInterface.h"
#include "ManagerThreadTaskInterface.h"
#include "ThreadSubject.h"
#include "ThreadConnection.h"
#include "ThreadInterfaces.h"
#include "ThreadConstants.h"
#include "ThreadKeyManager.h"
#include "ThreadMessages/ExchangeInterfaceMessages.h"
#include "ThreadMessages/LoggingMessages.h"
#include "ThreadMessages/ThreadManagementMessages.h"
#include "MessageHandling/ThreadMessageHandler.h"
#include "ThreadMessageFrame.h"
#include "TaskScheduler/TaskScheduler.h"
#include "TaskScheduler/ScheduledTask.h"
#include "Time/TimeKeeper.h"
#include "Time/TimeType.h"
#include "Time/TimeUtils.h"
#include "Time/TickTime.h"
#include "tbb/task_scheduler_init.h"
#include "tbb/task.h"
#include "ThreadStatics.h"
#include "ThreadTaskExecutionContext.h"
#include "PlatformTime.h"
#include "PlatformProcess.h"


typedef FastDelegate2< const SThreadKey &, double, void > ExecuteThreadDelegateType;

// A scheduled task that triggers the execution of a thread's service function by TBB
class CExecuteThreadScheduledTask : public CScheduledTask
{
	public:

		typedef CScheduledTask BASECLASS;

		CExecuteThreadScheduledTask( const ExecuteThreadDelegateType &execute_delegate, const SThreadKey &key, double execute_time_seconds ) :
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

		ExecuteThreadDelegateType ExecuteDelegate;
		SThreadKey Key;
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// A TBB task that executes a thread's service function
class CServiceThreadTBBTask : public tbb::task
{
	public:

		typedef tbb::task BASECLASS;

		CServiceThreadTBBTask( const shared_ptr< IManagerThreadTask > &thread_task, double elapsed_seconds ) :
			ThreadTask( thread_task ),
			ElapsedSeconds( elapsed_seconds )
		{}

		virtual ~CServiceThreadTBBTask() {}

		virtual tbb::task *execute( void )
		{
			CThreadTaskExecutionContext context( this );

			FATAL_ASSERT( CThreadStatics::Get_Current_Thread_Task() == nullptr );

			CThreadStatics::Set_Current_Thread_Task( ThreadTask.get() );
			ThreadTask->Service( ElapsedSeconds, context );
			CThreadStatics::Set_Current_Thread_Task( nullptr );
			ThreadTask->Flush_Partitioned_Messages();

			return nullptr;
		}

	private:

		shared_ptr< IManagerThreadTask > ThreadTask;

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
			CThreadTaskExecutionContext context( this );

			FATAL_ASSERT( CThreadStatics::Get_Current_Thread_Task() == nullptr );

			CLogInterface::Service_Logging( ElapsedSeconds, context );

			return nullptr;
		}

	private:

		double ElapsedSeconds;
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

enum EThreadTaskState
{
	ETTS_INITIALIZING,
	ETTS_RUNNING,
	ETTS_SHUTTING_DOWN_PHASE1,
	ETTS_SHUTTING_DOWN_PHASE2
};

// An internal class that tracks state about a thread task
class CThreadTaskRecord
{
	public:

		// Construction/destruction
		CThreadTaskRecord( const shared_ptr< IManagerThreadTask > &thread_task, const ExecuteThreadDelegateType &execute_delegate );
		CThreadTaskRecord( const SThreadKey &key );
		~CThreadTaskRecord();

		// Accessors
		const SThreadKey &Get_Key( void ) const { return ThreadTask->Get_Key(); }

		shared_ptr< IManagerThreadTask > Get_Thread_Task( void ) const { return ThreadTask; }

		EThreadTaskState Get_State( void ) const { return State; }
		void Set_State( EThreadTaskState state ) { State = state; }

		CThreadConnection *Get_Mailbox( void ) const { return Mailbox.get(); }

		// Operations
		void Add_Execute_Task( const shared_ptr< CTaskScheduler > &task_scheduler, double execution_time );
		void Remove_Execute_Task( const shared_ptr< CTaskScheduler > &task_scheduler );

		void Add_Pending_Shutdown_Key( const SThreadKey &key ) { PendingShutdownKeys.insert( key ); }
		void Remove_Pending_Shutdown_Key( const SThreadKey &key ) { PendingShutdownKeys.erase( key ); }

		// Queries
		bool Is_Shutting_Down( void ) const { return State == ETTS_SHUTTING_DOWN_PHASE1 || State == ETTS_SHUTTING_DOWN_PHASE2; }

		bool Has_Pending_Shutdown_Keys( void ) const { return PendingShutdownKeys.size() > 0; }

	private:

		SThreadKey Key;

		shared_ptr< IManagerThreadTask > ThreadTask;

		scoped_ptr< CThreadConnection > Mailbox;

		shared_ptr< CExecuteThreadScheduledTask > ExecuteTask;

		ExecuteThreadDelegateType ExecuteDelegate;

		EThreadTaskState State;

		std::set< SThreadKey, SThreadKeyContainerHelper > PendingShutdownKeys;
};

/**********************************************************************************************************************
	CThreadTaskRecord::CThreadTaskRecord -- constructor

		thread_task -- pointer to the thread task this record tracks
		execute_delegate -- the function to invoke when it's time to execute the thread through a TBB task
					
**********************************************************************************************************************/
CThreadTaskRecord::CThreadTaskRecord( const shared_ptr< IManagerThreadTask > &thread_task, const ExecuteThreadDelegateType &execute_delegate ) :
	Key( thread_task->Get_Key() ),
	ThreadTask( thread_task ),
	Mailbox( new CThreadConnection( thread_task->Get_Key() ) ),
	ExecuteTask( nullptr ),
	ExecuteDelegate( execute_delegate ),
	State( ETTS_INITIALIZING ),
	PendingShutdownKeys()
{
}

/**********************************************************************************************************************
	CThreadTaskRecord::CThreadTaskRecord -- alternative dummy constructor, only used to proxy the manager

		key -- key of the proxy thread
					
**********************************************************************************************************************/
CThreadTaskRecord::CThreadTaskRecord( const SThreadKey &key ) :
	Key( key ),
	ThreadTask( nullptr ),
	Mailbox( new CThreadConnection( key ) ),
	ExecuteTask( nullptr ),
	ExecuteDelegate(),
	State( ETTS_INITIALIZING ),
	PendingShutdownKeys()
{
	FATAL_ASSERT( key == MANAGER_THREAD_KEY );
}

/**********************************************************************************************************************
	CThreadTaskRecord::~CThreadTaskRecord -- destructor
					
**********************************************************************************************************************/
CThreadTaskRecord::~CThreadTaskRecord()
{
	FATAL_ASSERT( ExecuteTask == nullptr || !ExecuteTask->Is_Scheduled() );
}

/**********************************************************************************************************************
	CThreadTaskRecord::Add_Execute_Task -- Schedules an execution task for this thread at a requested time

		task_scheduler -- task scheduler to schedule the task in
		execution_time -- time the thread should be serviced
					
**********************************************************************************************************************/
void CThreadTaskRecord::Add_Execute_Task( const shared_ptr< CTaskScheduler > &task_scheduler, double execution_time )
{
	if ( ExecuteTask == nullptr )
	{
		ExecuteTask.reset( new CExecuteThreadScheduledTask( ExecuteDelegate, Get_Key(), execution_time ) );
	}
	else
	{
		FATAL_ASSERT( !ExecuteTask->Is_Scheduled() );

		ExecuteTask->Set_Execute_Time( execution_time );
	}

	task_scheduler->Submit_Task( ExecuteTask );
}

/**********************************************************************************************************************
	CThreadTaskRecord::Remove_Execute_Task -- removes a thread's execution task from a task scheduler

		task_scheduler -- task scheduler to remove the task from
					
**********************************************************************************************************************/
void CThreadTaskRecord::Remove_Execute_Task( const shared_ptr< CTaskScheduler > &task_scheduler )
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
	ThreadRecords(),
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

	CThreadStatics::Set_Concurrency_Manager( this );
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

	FATAL_ASSERT( ThreadRecords.size() == 0 );

	CLogInterface::Shutdown_Dynamic();
	CThreadStatics::Set_Concurrency_Manager( nullptr );

	State = ECMS_FINISHED;
}

/**********************************************************************************************************************
	CConcurrencyManager::Get_Record -- gets the thread task record for a given thread key

		key -- key of the thread to get the task record for

		Returns: pointer to the thread task record, or null
					
**********************************************************************************************************************/
shared_ptr< CThreadTaskRecord > CConcurrencyManager::Get_Record( const SThreadKey &key ) const
{
	auto iter = ThreadRecords.find( key );
	if ( iter != ThreadRecords.end() )
	{
		return iter->second;
	}

	return nullptr;
}

/**********************************************************************************************************************
	CConcurrencyManager::Get_Thread_Task -- gets the thread task for a given thread key

		key -- key of the thread to get the task for

		Returns: pointer to the thread task, or null
					
**********************************************************************************************************************/
shared_ptr< IManagerThreadTask > CConcurrencyManager::Get_Thread_Task( const SThreadKey &key ) const
{
	shared_ptr< CThreadTaskRecord > record = Get_Record( key );
	if ( record != nullptr )
	{
		return record->Get_Thread_Task();
	}

	return shared_ptr< IManagerThreadTask >( nullptr );
}

/**********************************************************************************************************************
	CConcurrencyManager::Run -- starts the concurrency manager system

		starting_thread -- initial thread task
					
**********************************************************************************************************************/
void CConcurrencyManager::Run( const shared_ptr< IManagerThreadTask > &starting_thread )
{
	Setup_For_Run( starting_thread );

	Service();
}

/**********************************************************************************************************************
	CConcurrencyManager::Setup_For_Run -- prepares the manager to begin its service loop

		starting_thread -- initial thread task
					
**********************************************************************************************************************/
void CConcurrencyManager::Setup_For_Run( const shared_ptr< IManagerThreadTask > &starting_thread )
{
	FATAL_ASSERT( State == ECMS_INITIALIZED );
	FATAL_ASSERT( ThreadRecords.size() == 0 );

	// make a proxy for the manager
	ThreadRecords[ MANAGER_THREAD_KEY ] = shared_ptr< CThreadTaskRecord >( new CThreadTaskRecord( MANAGER_THREAD_KEY ) );

	// all threads should receive an interface to the log thread automatically
	Handle_Push_Interface_Request( MANAGER_THREAD_KEY, shared_ptr< CPushInterfaceRequest >( new CPushInterfaceRequest( LOG_THREAD_KEY, ALL_THREAD_KEY ) ) );

	// Setup the logging thread and the initial thread
	Add_Thread( CLogInterface::Get_Logging_Thread() );
	Add_Thread( shared_ptr< IManagerThreadTask>( starting_thread ) );

	// Reset time
	TimeKeeper->Set_Base_Time( TT_REAL_TIME, STickTime( CPlatformTime::Get_High_Resolution_Time() ) );
	TimeKeeper->Set_Base_Time( TT_GAME_TIME, STickTime( 0 ) );

	State = ECMS_RUNNING;
}

/**********************************************************************************************************************
	CConcurrencyManager::Add_Thread -- adds a new thread task into the concurrency system

		thread -- thread task to add to the concurrency system
					
**********************************************************************************************************************/
void CConcurrencyManager::Add_Thread( const shared_ptr< IManagerThreadTask > &thread )
{
	FATAL_ASSERT( thread->Get_Key().Get_Thread_Subject() != TS_CONCURRENCY_MANAGER );

	// perform any sub key allocation if necessary
	SThreadKey new_key = KeyManager->Fill_In_Thread_Key( thread->Get_Key() );
	FATAL_ASSERT( new_key.Is_Valid() );
	FATAL_ASSERT( ThreadRecords.find( new_key ) == ThreadRecords.end() );

	thread->Set_Key( new_key );
	KeyManager->Add_Tracked_Thread_Key( new_key ); 

	thread->Initialize();

	// track the thread task in a task record
	shared_ptr< CThreadTaskRecord > thread_record( new CThreadTaskRecord( thread, ExecuteThreadDelegateType( this, &CConcurrencyManager::Execute_Thread_Task ) ) );
	ThreadRecords[ new_key ] = thread_record;

	// Interface setup
	CThreadConnection *thread_connection = thread_record->Get_Mailbox();
	Handle_Ongoing_Interface_Requests( thread_connection );

	thread->Set_Manager_Interface( Get_Write_Interface( MANAGER_THREAD_KEY ) );
	thread->Set_Read_Interface( thread_connection->Get_Reader_Interface() );

	// Schedule first execution if necessary
	if ( thread->Is_Root_Thread() )
	{
		thread_record->Add_Execute_Task( Get_Task_Scheduler( thread->Get_Time_Type() ), TimeKeeper->Get_Elapsed_Seconds( thread->Get_Time_Type() ) );
	}
}

/**********************************************************************************************************************
	CConcurrencyManager::Handle_Ongoing_Interface_Requests -- when a thread is added, this function pushes all
		interfaces that should be given to the new thread as well as sends this thread's interface to everyone
		who has an outstanding request for its interface

		thread_connection -- handle to a thread's read and write interfaces
					
**********************************************************************************************************************/
void CConcurrencyManager::Handle_Ongoing_Interface_Requests( CThreadConnection *thread_connection )
{
	const SThreadKey &new_key = thread_connection->Get_Key();
	FATAL_ASSERT( !Is_Thread_Shutting_Down( new_key ) );

	// persistent push requests
	for ( auto iter = PersistentPushRequests.cbegin(); iter != PersistentPushRequests.cend(); ++iter )
	{
		const shared_ptr< const CPushInterfaceRequest > &push_request = *iter;
		const SThreadKey &source_key = push_request->Get_Source_Key();
		if ( push_request->Get_Target_Key().Matches( new_key ) && source_key != new_key && !Is_Thread_Shutting_Down( source_key ) )
		{
			shared_ptr< const IThreadMessage > message( new CAddInterfaceMessage( source_key, Get_Write_Interface( source_key ) ) );
			Send_Thread_Message( new_key, message );
		}
	}

	// persistent get requests
	for ( auto iter = PersistentGetRequests.cbegin(); iter != PersistentGetRequests.cend(); ++iter )
	{
		const shared_ptr< const CGetInterfaceRequest > &get_request = *iter;
		const SThreadKey &source_key = get_request->Get_Source_Key();
		if ( get_request->Get_Target_Key().Matches( new_key ) && source_key != new_key && !Is_Thread_Shutting_Down( source_key ) )
		{
			shared_ptr< const IThreadMessage > message( new CAddInterfaceMessage( new_key, Get_Write_Interface( new_key ) ) );
			Send_Thread_Message( source_key, message );
		}
	}

	// unfulfilled single-targeted get requests
	auto push_range = UnfulfilledPushRequests.equal_range( new_key );
	for ( auto iter = push_range.first; iter != push_range.second; ++iter )
	{
		const shared_ptr< const CPushInterfaceRequest > &push_request = iter->second;
		const SThreadKey &source_key = push_request->Get_Source_Key();

		if ( !Is_Thread_Shutting_Down( source_key ) )
		{
			shared_ptr< const IThreadMessage > message( new CAddInterfaceMessage( source_key, Get_Write_Interface( source_key ) ) );
			Send_Thread_Message( new_key, message );
		}
	}

	UnfulfilledPushRequests.erase( push_range.first, push_range.second );

	// unfulfilled get requests
	auto get_range = UnfulfilledGetRequests.equal_range( new_key );
	for ( auto iter = get_range.first; iter != get_range.second; ++iter )
	{
		const shared_ptr< const CGetInterfaceRequest > &get_request = iter->second;
		const SThreadKey &source_key = get_request->Get_Source_Key();

		if ( !Is_Thread_Shutting_Down( source_key ) )
		{
			shared_ptr< const IThreadMessage > message( new CAddInterfaceMessage( new_key, Get_Write_Interface( new_key ) ) );
			Send_Thread_Message( source_key, message );
		}
	}

	UnfulfilledGetRequests.erase( get_range.first, get_range.second );
}

/**********************************************************************************************************************
	CConcurrencyManager::Get_Write_Interface -- gets the write-only interface to a thread task

		key -- key of the thread task to get the interface for

		Returns: a pointer to the write-only interface, or null
					
**********************************************************************************************************************/
shared_ptr< CWriteOnlyThreadInterface > CConcurrencyManager::Get_Write_Interface( const SThreadKey &key ) const
{
	auto iter = ThreadRecords.find( key );
	if ( iter != ThreadRecords.end() )
	{
		return iter->second->Get_Mailbox()->Get_Writer_Interface();
	}

	return shared_ptr< CWriteOnlyThreadInterface >( nullptr );
}

/**********************************************************************************************************************
	CConcurrencyManager::Get_Self_Read_Interface -- gets the read-only interface of the manager

		Returns: a pointer to the read-only interface of the manager
					
**********************************************************************************************************************/
shared_ptr< CReadOnlyThreadInterface > CConcurrencyManager::Get_Self_Read_Interface( void ) const
{
	auto iter = ThreadRecords.find( MANAGER_THREAD_KEY );
	FATAL_ASSERT( iter != ThreadRecords.end() );

	return iter->second->Get_Mailbox()->Get_Reader_Interface();
}

/**********************************************************************************************************************
	CConcurrencyManager::Send_Thread_Message -- queues up a thread message to a thread task

		dest_key -- the message's destination thread
		message -- message to send
					
**********************************************************************************************************************/
void CConcurrencyManager::Send_Thread_Message( const SThreadKey &dest_key, const shared_ptr< const IThreadMessage > &message )
{
	auto iter = PendingOutboundFrames.find( dest_key );
	if ( iter == PendingOutboundFrames.end() )
	{
		shared_ptr< CThreadMessageFrame > frame( new CThreadMessageFrame( MANAGER_THREAD_KEY ) );
		frame->Add_Message( message );
		PendingOutboundFrames.insert( FrameTableType::value_type( dest_key, frame ) );
		return;
	}

	iter->second->Add_Message( message );
}

/**********************************************************************************************************************
	CConcurrencyManager::Flush_Frames -- sends all outbound message frames to their respective thread destinations
					
**********************************************************************************************************************/
void CConcurrencyManager::Flush_Frames( void )
{
	std::vector< SThreadKey > sent_frames;

	for ( auto frame_iterator = PendingOutboundFrames.cbegin(); frame_iterator != PendingOutboundFrames.cend(); ++frame_iterator )
	{
		shared_ptr< CWriteOnlyThreadInterface > write_interface = Get_Write_Interface( frame_iterator->first );
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
	while ( ThreadRecords.size() > 0 )
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
	if ( ThreadRecords.size() == 2 && State != ECMS_SHUTTING_DOWN_PHASE2 )
	{
		// Nothing left but the log thread and our own proxy thread
		FATAL_ASSERT( Get_Record( MANAGER_THREAD_KEY ) != nullptr && Get_Record( LOG_THREAD_KEY ) != nullptr );

		State = ECMS_SHUTTING_DOWN_PHASE2;

		shared_ptr< const IThreadMessage > message( new CShutdownThreadRequest( true ) );
		Send_Thread_Message( LOG_THREAD_KEY, message );
	}
	else if ( ThreadRecords.size() == 1 )
	{
		// Log thread now gone, we're ready to quit
		FATAL_ASSERT( Get_Record( MANAGER_THREAD_KEY ) != nullptr );

		ThreadRecords.clear();
	}
}

/**********************************************************************************************************************
	CConcurrencyManager::Service_Incoming_Frames -- iterates all incoming message frames and handles the messages
		within them
					
**********************************************************************************************************************/
void CConcurrencyManager::Service_Incoming_Frames( void )
{
	std::vector< shared_ptr< CThreadMessageFrame > > control_frames;
	Get_Self_Read_Interface()->Remove_Frames( control_frames );

	// iterate all frames
	for ( uint32 i = 0; i < control_frames.size(); ++i )
	{
		const shared_ptr< CThreadMessageFrame > &frame = control_frames[ i ];
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
void CConcurrencyManager::Handle_Message( const SThreadKey &key, const shared_ptr< const IThreadMessage > &message )
{
	const IThreadMessage *msg_base = message.get();

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
	REGISTER_THIS_HANDLER( CGetInterfaceRequest, CConcurrencyManager, Handle_Get_Interface_Request )
	REGISTER_THIS_HANDLER( CPushInterfaceRequest, CConcurrencyManager, Handle_Push_Interface_Request )
	REGISTER_THIS_HANDLER( CAddThreadMessage, CConcurrencyManager, Handle_Add_Thread_Message )
	REGISTER_THIS_HANDLER( CShutdownThreadMessage, CConcurrencyManager, Handle_Shutdown_Thread_Message )
	REGISTER_THIS_HANDLER( CRescheduleThreadMessage, CConcurrencyManager, Handle_Reschedule_Thread_Message )
	REGISTER_THIS_HANDLER( CShutdownInterfaceAcknowledgement, CConcurrencyManager, Handle_Shutdown_Interface_Acknowledgement )
	REGISTER_THIS_HANDLER( CShutdownThreadAcknowledgement, CConcurrencyManager, Handle_Shutdown_Thread_Acknowledgement )
	REGISTER_THIS_HANDLER( CShutdownManagerMessage, CConcurrencyManager, Handle_Shutdown_Manager_Message )
} 

/**********************************************************************************************************************
	CConcurrencyManager::Register_Handler -- registers a single message handler for a message type

		message_type_info -- C++ type info for the message class
		handler -- message handling delegate to invoke for this message type
					
**********************************************************************************************************************/
void CConcurrencyManager::Register_Handler( const std::type_info &message_type_info, const shared_ptr< IThreadMessageHandler > &handler )
{
	Loki::TypeInfo key( message_type_info );

	FATAL_ASSERT( MessageHandlers.find( key ) == MessageHandlers.end() );
	MessageHandlers[ key ] = handler;
}

/**********************************************************************************************************************
	CConcurrencyManager::Handle_Get_Interface_Request -- message handler for a GetInterface request

		key -- thread source of the request
		message -- the get interface request
					
**********************************************************************************************************************/
void CConcurrencyManager::Handle_Get_Interface_Request( const SThreadKey & /*key*/, const shared_ptr< const CGetInterfaceRequest > &message )
{
	// don't handle messages while shutting down
	if ( Is_Manager_Shutting_Down() )
	{
		return;
	}

	// don't give out interfaces when a thread is shutting down
	const SThreadKey &requested_key = message->Get_Target_Key();
	const SThreadKey &source_key = message->Get_Source_Key();
	if ( Is_Thread_Shutting_Down( source_key ) )
	{
		return;
	}

	if ( requested_key.Is_Unique() )
	{
		// it's a single-targeted request
		shared_ptr< CThreadTaskRecord > record = Get_Record( requested_key );
		if ( record == nullptr )
		{
			// not yet fulfillable, track it until it can be fulfilled
			UnfulfilledGetRequests.insert( GetRequestCollectionType::value_type( requested_key, message ) );
		}
		else if ( !record->Is_Shutting_Down() )
		{
			// fulfill the request
			shared_ptr< const IThreadMessage > message( new CAddInterfaceMessage( requested_key, record->Get_Mailbox()->Get_Writer_Interface() ) );
			Send_Thread_Message( source_key, message );
		}
	}
	else
	{
		// it's a persistent pattern-matching request, match all existing threads and track against future adds
		for ( auto iter = ThreadRecords.cbegin(); iter != ThreadRecords.cend(); ++iter )
		{
			if ( message->Get_Target_Key().Matches( iter->first ) && !Is_Thread_Shutting_Down( iter->first ) )
			{
				shared_ptr< const IThreadMessage > message( new CAddInterfaceMessage( iter->first, Get_Write_Interface( iter->first ) ) );
				Send_Thread_Message( source_key, message );
			}
		}

		PersistentGetRequests.push_back( message );
	}
}

/**********************************************************************************************************************
	CConcurrencyManager::Handle_Push_Interface_Request -- message handler for a PushInterface request

		key -- thread source of the request
		message -- the push interface request
					
**********************************************************************************************************************/
void CConcurrencyManager::Handle_Push_Interface_Request( const SThreadKey &key, const shared_ptr< const CPushInterfaceRequest > &message )
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
	if ( Is_Thread_Shutting_Down( source_key ) )
	{
		return;
	}

	if ( target_key.Is_Unique() )
	{
		// single target request
		if ( ThreadRecords.find( target_key ) != ThreadRecords.end() )
		{
			// it's fulfillable now, do it if the target is not shutting down
			if ( !Is_Thread_Shutting_Down( target_key ) )
			{
				shared_ptr< const IThreadMessage > response_message( new CAddInterfaceMessage( source_key, Get_Write_Interface( source_key ) ) );
				Send_Thread_Message( target_key, response_message );
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
		for ( auto iter = ThreadRecords.cbegin(); iter != ThreadRecords.cend(); ++iter )
		{
			if ( target_key.Matches( iter->first ) && !Is_Thread_Shutting_Down( iter->first ) )
			{
				shared_ptr< const IThreadMessage > response_message( new CAddInterfaceMessage( source_key, Get_Write_Interface( source_key ) ) );
				Send_Thread_Message( iter->first, response_message );
			}
		}

		PersistentPushRequests.push_back( message );
	}
}

/**********************************************************************************************************************
	CConcurrencyManager::Handle_Add_Thread_Message -- message handler for an AddThread request

		key -- thread source of the request
		message -- the add thread request
					
**********************************************************************************************************************/
void CConcurrencyManager::Handle_Add_Thread_Message( const SThreadKey &key, const shared_ptr< const CAddThreadMessage > &message )
{
	const SThreadKey &new_key = message->Get_Thread_Task()->Get_Key();
	FATAL_ASSERT( !Is_Thread_Shutting_Down( new_key ) );

	if ( Is_Manager_Shutting_Down() )
	{
		return;
	}

	Add_Thread( static_pointer_cast< IManagerThreadTask >( message->Get_Thread_Task() ) );

	// return interface, push interface options
	if ( !Is_Thread_Shutting_Down( key ) )
	{
		if ( message->Should_Return_Interface() )
		{
			shared_ptr< const IThreadMessage > response_message( new CAddInterfaceMessage( new_key, Get_Write_Interface( new_key ) ) );
			Send_Thread_Message( key, response_message );
		}

		if ( message->Should_Forward_Creator_Interface() )
		{
			shared_ptr< const IThreadMessage > response_message( new CAddInterfaceMessage( key, Get_Write_Interface( key ) ) );
			Send_Thread_Message( message->Get_Thread_Task()->Get_Key(), response_message );
		}
	}
}

/**********************************************************************************************************************
	CConcurrencyManager::Handle_Shutdown_Thread_Message -- message handler for an ShutdownThread request

		key -- thread source of the request
		message -- the shutdown thread request
					
**********************************************************************************************************************/
void CConcurrencyManager::Handle_Shutdown_Thread_Message( const SThreadKey & /*key*/, const shared_ptr< const CShutdownThreadMessage > &message )
{
	if ( Is_Manager_Shutting_Down() )
	{
		return;
	}

	Initiate_Thread_Shutdown( message->Get_Key() );
}

/**********************************************************************************************************************
	CConcurrencyManager::Handle_Reschedule_Thread_Message -- message handler for a RescheduleThread request

		key -- thread source of the request
		message -- the reschedule thread request
					
**********************************************************************************************************************/
void CConcurrencyManager::Handle_Reschedule_Thread_Message( const SThreadKey & /*key*/, const shared_ptr< const CRescheduleThreadMessage > &message )
{
	const SThreadKey &rescheduled_key = message->Get_Key();
	shared_ptr< CThreadTaskRecord > record = Get_Record( rescheduled_key );
	if ( record == nullptr )
	{
		return;
	}

	ETimeType time_type = record->Get_Thread_Task()->Get_Time_Type();
	double execute_time = std::min( message->Get_Reschedule_Time(), TimeKeeper->Get_Elapsed_Seconds( time_type ) );
	record->Add_Execute_Task( Get_Task_Scheduler( time_type ), execute_time );
}

/**********************************************************************************************************************
	CConcurrencyManager::Handle_Shutdown_Interface_Acknowledgement -- message handler for a ShutdownInterface 
		acknowledgement

		key -- thread source of the acknowledgement
		message -- the shutdown interface acknowledgement
					
**********************************************************************************************************************/
void CConcurrencyManager::Handle_Shutdown_Interface_Acknowledgement( const SThreadKey &key, const shared_ptr< const CShutdownInterfaceAcknowledgement > &message )
{
	if ( Is_Manager_Shutting_Down() )
	{
		return;
	}

	const SThreadKey &shutdown_key = message->Get_Shutdown_Key();
	shared_ptr< CThreadTaskRecord > record = Get_Record( shutdown_key );
	if ( record == nullptr )
	{
		return;
	}

	FATAL_ASSERT( record->Get_State() == ETTS_SHUTTING_DOWN_PHASE1 );

	record->Remove_Pending_Shutdown_Key( key );
	if ( !record->Has_Pending_Shutdown_Keys() )
	{
		// we've heard back from everyone; no one has a handle to this thread anymore, so we can tell it to shut down
		record->Set_State( ETTS_SHUTTING_DOWN_PHASE2 );

		shared_ptr< const IThreadMessage > shutdown_request( new CShutdownThreadRequest( false ) );
		Send_Thread_Message( shutdown_key, shutdown_request );
	}
}

/**********************************************************************************************************************
	CConcurrencyManager::Handle_Shutdown_Thread_Acknowledgement -- message handler for a ShutdownThread 
		acknowledgement

		key -- thread source of the acknowledgement
		message -- the shutdown thread acknowledgement
					
**********************************************************************************************************************/
void CConcurrencyManager::Handle_Shutdown_Thread_Acknowledgement( const SThreadKey &key, const shared_ptr< const CShutdownThreadAcknowledgement > & /*message*/ )
{
	auto iter = ThreadRecords.find( key );
	FATAL_ASSERT( iter != ThreadRecords.end() );
	FATAL_ASSERT( iter->second->Get_State() == ETTS_SHUTTING_DOWN_PHASE2 || Is_Manager_Shutting_Down() );

	ThreadRecords.erase( iter );

	KeyManager->Remove_Tracked_Thread_Key( key );
}

/**********************************************************************************************************************
	CConcurrencyManager::Handle_Shutdown_Thread_Acknowledgement -- message handler for a ShutdownManager message

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
	shared_ptr< const IThreadMessage > shutdown_thread_msg( new CShutdownThreadRequest( true ) );
	for ( auto iter = ThreadRecords.cbegin(); iter != ThreadRecords.cend(); ++iter )
	{
		if ( iter->first == MANAGER_THREAD_KEY || iter->first == LOG_THREAD_KEY )
		{
			continue;
		}

		Send_Thread_Message( iter->first, shutdown_thread_msg );
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
	CConcurrencyManager::Execute_Thread_Task -- queues up a tbb task to execute the supplied thread task's service

		key -- key of the thread task to have executed
		current_time_seconds -- current time from the thread task's standpoint
					
**********************************************************************************************************************/
void CConcurrencyManager::Execute_Thread_Task( const SThreadKey &key, double current_time_seconds )
{
	auto iter = ThreadRecords.find( key );
	if ( iter == ThreadRecords.end() )
	{
		return;
	}

	shared_ptr< CThreadTaskRecord > record = iter->second;
	if ( record->Get_State() == ETTS_INITIALIZING )
	{
		record->Set_State( ETTS_RUNNING );
	}

	shared_ptr< IManagerThreadTask > thread_task_base = record->Get_Thread_Task();

	if ( key == LOG_THREAD_KEY )
	{
		CServiceLoggingThreadTBBTask &tbb_task = *new( tbb::task::allocate_root() ) CServiceLoggingThreadTBBTask( current_time_seconds );
		tbb::task::enqueue( tbb_task );
	}
	else
	{
		CServiceThreadTBBTask &tbb_task = *new( tbb::task::allocate_root() ) CServiceThreadTBBTask( thread_task_base, current_time_seconds );
		tbb::task::enqueue( tbb_task );
	}
}

/**********************************************************************************************************************
	CConcurrencyManager::Initiate_Thread_Shutdown -- guides a thread task into the shutdown process

		key -- key of the thread task to start shutting down
					
**********************************************************************************************************************/
void CConcurrencyManager::Initiate_Thread_Shutdown( const SThreadKey &key )
{
	FATAL_ASSERT( key.Is_Unique() );
	FATAL_ASSERT( key != MANAGER_THREAD_KEY && key != LOG_THREAD_KEY );

	shared_ptr< CThreadTaskRecord > shutdown_record = Get_Record( key );
	if ( shutdown_record == nullptr || shutdown_record->Is_Shutting_Down() )
	{
		return;
	}

	shutdown_record->Set_State( ETTS_SHUTTING_DOWN_PHASE1 );

	// this message can be shared and broadcast
	shared_ptr< const IThreadMessage > shutdown_interface_msg( new CShutdownInterfaceMessage( key ) );
	for ( auto iter = ThreadRecords.cbegin(); iter != ThreadRecords.cend(); ++iter )
	{
		if ( key == iter->first || iter->first == MANAGER_THREAD_KEY || iter->first == LOG_THREAD_KEY )
		{
			continue;
		}

		shutdown_record->Add_Pending_Shutdown_Key( iter->first );
		Send_Thread_Message( iter->first, shutdown_interface_msg );
	}

	// Remove all push/get requests related to this thread
	Clear_Related_Interface_Requests( key );

	// Move to the next state if we're not waiting on any other thread acknowledgements
	if ( !shutdown_record->Has_Pending_Shutdown_Keys() )
	{
		shutdown_record->Set_State( ETTS_SHUTTING_DOWN_PHASE2 );

		shared_ptr< const IThreadMessage > shutdown_request( new CShutdownThreadRequest( false ) );
		Send_Thread_Message( key, shutdown_request );
	}
}

/**********************************************************************************************************************
	CConcurrencyManager::Clear_Related_Interface_Requests -- clears interface requests related to a thread

		key -- key of the thread task to clear interface requests about
					
**********************************************************************************************************************/
void CConcurrencyManager::Clear_Related_Interface_Requests( const SThreadKey &key )
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
	CConcurrencyManager::Is_Thread_Shutting_Down -- is the supplied thread in the process of shutting down

		key -- key of the thread task to check shutdown status of

		Returns: true if the thread is shutting down, false otherwise
					
**********************************************************************************************************************/
bool CConcurrencyManager::Is_Thread_Shutting_Down( const SThreadKey &key ) const
{
	shared_ptr< CThreadTaskRecord > record = Get_Record( key );
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
		Send_Thread_Message( LOG_THREAD_KEY, shared_ptr< const IThreadMessage >( new CLogRequestMessage( MANAGER_THREAD_KEY, message ) ) );
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
