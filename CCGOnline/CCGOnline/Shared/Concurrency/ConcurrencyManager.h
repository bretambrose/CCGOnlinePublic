/**********************************************************************************************************************

	ConcurrencyManager.h
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

#ifndef CONCURRENCY_MANAGER_H
#define CONCURRENCY_MANAGER_H

#include "ThreadKey.h"
#include "TypeInfoUtils.h"

class IManagedVirtualProcess;
class CReadOnlyMailbox;
class CVirtualProcessMailbox;
class CPushMailboxRequest;
class CGetMailboxRequest;
class CAddNewVirtualProcessMessage;
class CShutdownVirtualProcessMessage;
class CRescheduleVirtualProcessMessage;
class CReleaseMailboxResponse;
class CShutdownSelfResponse;
class CShutdownManagerMessage;
class CWriteOnlyMailbox;
class IVirtualProcessMessage;
class IVirtualProcessMessageHandler;
class CVirtualProcessMessageFrame;
class CTaskScheduler;
class CTimeKeeper;
class CVirtualProcessRecord;
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

		void Run( const shared_ptr< IManagedVirtualProcess > &starting_process );

		void Log( const std::wstring &message );

	private:

		friend class CConcurrencyManagerTester;

		// Accessors
		shared_ptr< CVirtualProcessRecord > Get_Record( const SThreadKey &key ) const;

		shared_ptr< IManagedVirtualProcess > Get_Virtual_Process( const SThreadKey &key ) const;

		shared_ptr< CWriteOnlyMailbox > Get_Mailbox( const SThreadKey &key ) const;
		shared_ptr< CReadOnlyMailbox > Get_My_Mailbox( void ) const;

		shared_ptr< CTaskScheduler > Get_Task_Scheduler( ETimeType time_type ) const;

		// Message handling
		void Register_Message_Handlers( void );
		void Register_Handler( const std::type_info &message_type_info, const shared_ptr< IVirtualProcessMessageHandler > &handler );

		void Handle_Message( const SThreadKey &key, const shared_ptr< const IVirtualProcessMessage > &message );
		void Handle_Get_Mailbox_Request( const SThreadKey &key, const shared_ptr< const CGetMailboxRequest > &message );
		void Handle_Push_Mailbox_Request( const SThreadKey &key, const shared_ptr< const CPushMailboxRequest > &message );
		void Handle_Add_New_Virtual_Process_Message( const SThreadKey &key, const shared_ptr< const CAddNewVirtualProcessMessage > &message );
		void Handle_Shutdown_Virtual_Process_Message( const SThreadKey &key, const shared_ptr< const CShutdownVirtualProcessMessage > &message );
		void Handle_Reschedule_Virtual_Process_Message( const SThreadKey &key, const shared_ptr< const CRescheduleVirtualProcessMessage > &message );
		void Handle_Release_Mailbox_Response( const SThreadKey &key, const shared_ptr< const CReleaseMailboxResponse > &message );
		void Handle_Shutdown_Self_Response( const SThreadKey &key, const shared_ptr< const CShutdownSelfResponse > &message );
		void Handle_Shutdown_Manager_Message( const SThreadKey &key, const shared_ptr< const CShutdownManagerMessage > &message );

		// Execution
		void Service( void );
		void Service_One_Iteration( void );
		void Service_Shutdown( void );
		void Service_Incoming_Frames( void );

		void Setup_For_Run( const shared_ptr< IManagedVirtualProcess > &starting_process );
		void Shutdown( void );

		// Execution helpers
		void Execute_Virtual_Process( const SThreadKey &key, double current_time_seconds );

		void Add_Virtual_Process( const shared_ptr< IManagedVirtualProcess > &process );

		void Send_Virtual_Process_Message( const SThreadKey &dest_key, const shared_ptr< const IVirtualProcessMessage > &message );
		void Flush_Frames( void );

		void Handle_Ongoing_Mailbox_Requests( CVirtualProcessMailbox *mailbox );
		void Clear_Related_Mailbox_Requests( const SThreadKey &key );

		// Shutdown helpers
		void Initiate_Process_Shutdown( const SThreadKey &key );
		bool Is_Process_Shutting_Down( const SThreadKey &key ) const;
		bool Is_Manager_Shutting_Down( void ) const;
		
		// Time management
		double Get_Game_Time( void ) const;
		void Set_Game_Time( double game_time_seconds );

		// Types
		typedef std::vector< shared_ptr< const CPushMailboxRequest > > PersistentPushRequestCollectionType;
		typedef std::vector< shared_ptr< const CGetMailboxRequest > > PersistentGetRequestCollectionType;
		typedef std::multimap< SThreadKey, shared_ptr< const CPushMailboxRequest >, SThreadKeyContainerHelper > PushRequestCollectionType;
		typedef std::multimap< SThreadKey, shared_ptr< const CGetMailboxRequest >, SThreadKeyContainerHelper > GetRequestCollectionType;

		typedef stdext::hash_map< SThreadKey, shared_ptr< CVirtualProcessMessageFrame >, SThreadKeyContainerHelper > FrameTableType;
		typedef stdext::hash_map< Loki::TypeInfo, shared_ptr< IVirtualProcessMessageHandler >, STypeInfoContainerHelper > VirtualProcessMessageHandlerTableType;
		typedef stdext::hash_map< SThreadKey, shared_ptr< CVirtualProcessRecord >, SThreadKeyContainerHelper > VirtualProcessRecordTableType;

		// Private Data
		VirtualProcessRecordTableType ProcessRecords;

		PushRequestCollectionType					UnfulfilledPushRequests;
		PersistentPushRequestCollectionType		PersistentPushRequests;
		GetRequestCollectionType					UnfulfilledGetRequests;
		PersistentGetRequestCollectionType		PersistentGetRequests;

		FrameTableType PendingOutboundFrames;

		VirtualProcessMessageHandlerTableType MessageHandlers;

		stdext::hash_map< ETimeType, shared_ptr< CTaskScheduler > > TaskSchedulers;
		scoped_ptr< CTimeKeeper > TimeKeeper;

		scoped_ptr< CThreadKeyManager > KeyManager;

		scoped_ptr< tbb::task_scheduler_init > TBBTaskSchedulerInit;

		EConcurrencyManagerState State;
};



#endif // CONCURRENCY_MANAGER_H
