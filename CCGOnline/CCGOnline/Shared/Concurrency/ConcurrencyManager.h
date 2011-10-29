/**********************************************************************************************************************

	[Placeholder for eventual source license]

	ConcurrencyManager.h
		A component definining the central, manager class that manages all thread tasks

	(c) Copyright 2011, Bret Ambrose.  All rights reserved.

**********************************************************************************************************************/

#ifndef CONCURRENCY_MANAGER_H
#define CONCURRENCY_MANAGER_H

#include "ThreadKey.h"
#include "TypeInfoUtils.h"

class IManagerThreadTask;
class CReadOnlyThreadInterface;
class CThreadConnection;
class CPushInterfaceRequest;
class CGetInterfaceRequest;
class CAddThreadMessage;
class CShutdownThreadMessage;
class CRescheduleThreadMessage;
class CShutdownInterfaceAcknowledgement;
class CShutdownThreadAcknowledgement;
class CShutdownManagerMessage;
class CWriteOnlyThreadInterface;
class IThreadMessage;
class IThreadMessageHandler;
class CThreadMessageFrame;
class CTaskScheduler;
class CTimeKeeper;
class CThreadTaskRecord;
class CThreadKeyManager;

struct STickTime;

enum ETimeType;
enum EConcurrencyManagerState;

namespace tbb
{
	class task_scheduler_init;
}

// the central, manager class that manages all thread tasks
class CConcurrencyManager
{
	public:

		// Construction/Destruction
		CConcurrencyManager( void );
		~CConcurrencyManager();

		// Public interface
		void Initialize( bool delete_all_logs );

		void Run( const shared_ptr< IManagerThreadTask > &starting_thread );

		void Log( const std::wstring &message );

	private:

		friend class CConcurrencyManagerTester;

		// Accessors
		shared_ptr< CThreadTaskRecord > Get_Record( const SThreadKey &key ) const;

		shared_ptr< IManagerThreadTask > Get_Thread_Task( const SThreadKey &key ) const;

		shared_ptr< CWriteOnlyThreadInterface > Get_Write_Interface( const SThreadKey &key ) const;
		shared_ptr< CReadOnlyThreadInterface > Get_Self_Read_Interface( void ) const;

		shared_ptr< CTaskScheduler > Get_Task_Scheduler( ETimeType time_type ) const;

		// Message handling
		void Register_Message_Handlers( void );
		void Register_Handler( const std::type_info &message_type_info, const shared_ptr< IThreadMessageHandler > &handler );

		void Handle_Message( const SThreadKey &key, const shared_ptr< const IThreadMessage > &message );
		void Handle_Get_Interface_Request( const SThreadKey &key, const shared_ptr< const CGetInterfaceRequest > &message );
		void Handle_Push_Interface_Request( const SThreadKey &key, const shared_ptr< const CPushInterfaceRequest > &message );
		void Handle_Add_Thread_Message( const SThreadKey &key, const shared_ptr< const CAddThreadMessage > &message );
		void Handle_Shutdown_Thread_Message( const SThreadKey &key, const shared_ptr< const CShutdownThreadMessage > &message );
		void Handle_Reschedule_Thread_Message( const SThreadKey &key, const shared_ptr< const CRescheduleThreadMessage > &message );
		void Handle_Shutdown_Interface_Acknowledgement( const SThreadKey &key, const shared_ptr< const CShutdownInterfaceAcknowledgement > &message );
		void Handle_Shutdown_Thread_Acknowledgement( const SThreadKey &key, const shared_ptr< const CShutdownThreadAcknowledgement > &message );
		void Handle_Shutdown_Manager_Message( const SThreadKey &key, const shared_ptr< const CShutdownManagerMessage > &message );

		// Execution
		void Service( void );
		void Service_One_Iteration( void );
		void Service_Shutdown( void );
		void Service_Incoming_Frames( void );

		void Setup_For_Run( const shared_ptr< IManagerThreadTask > &starting_thread );
		void Shutdown( void );

		// Execution helpers
		void Execute_Thread_Task( const SThreadKey &key, double current_time_seconds );

		void Add_Thread( const shared_ptr< IManagerThreadTask > &thread );

		void Send_Thread_Message( const SThreadKey &dest_key, const shared_ptr< const IThreadMessage > &message );
		void Flush_Frames( void );

		void Handle_Ongoing_Interface_Requests( CThreadConnection *thread_connection );
		void Clear_Related_Interface_Requests( const SThreadKey &key );

		// Shutdown helpers
		void Initiate_Thread_Shutdown( const SThreadKey &key );
		bool Is_Thread_Shutting_Down( const SThreadKey &key ) const;
		bool Is_Manager_Shutting_Down( void ) const;
		
		// Time management
		double Get_Game_Time( void ) const;
		void Set_Game_Time( double game_time_seconds );

		// Types
		typedef std::vector< shared_ptr< const CPushInterfaceRequest > > PersistentPushRequestCollectionType;
		typedef std::vector< shared_ptr< const CGetInterfaceRequest > > PersistentGetRequestCollectionType;
		typedef std::multimap< SThreadKey, shared_ptr< const CPushInterfaceRequest >, SThreadKeyContainerHelper > PushRequestCollectionType;
		typedef std::multimap< SThreadKey, shared_ptr< const CGetInterfaceRequest >, SThreadKeyContainerHelper > GetRequestCollectionType;

		typedef stdext::hash_map< SThreadKey, shared_ptr< CThreadMessageFrame >, SThreadKeyContainerHelper > FrameTableType;
		typedef stdext::hash_map< Loki::TypeInfo, shared_ptr< IThreadMessageHandler >, STypeInfoContainerHelper > ThreadMessageHandlerTableType;
		typedef stdext::hash_map< SThreadKey, shared_ptr< CThreadTaskRecord >, SThreadKeyContainerHelper > ThreadRecordTableType;

		// Private Data
		ThreadRecordTableType ThreadRecords;

		PushRequestCollectionType					UnfulfilledPushRequests;
		PersistentPushRequestCollectionType		PersistentPushRequests;
		GetRequestCollectionType					UnfulfilledGetRequests;
		PersistentGetRequestCollectionType		PersistentGetRequests;

		FrameTableType PendingOutboundFrames;

		ThreadMessageHandlerTableType MessageHandlers;

		stdext::hash_map< ETimeType, shared_ptr< CTaskScheduler > > TaskSchedulers;
		scoped_ptr< CTimeKeeper > TimeKeeper;

		scoped_ptr< CThreadKeyManager > KeyManager;

		scoped_ptr< tbb::task_scheduler_init > TBBTaskSchedulerInit;

		EConcurrencyManagerState State;
};



#endif // CONCURRENCY_MANAGER_H
