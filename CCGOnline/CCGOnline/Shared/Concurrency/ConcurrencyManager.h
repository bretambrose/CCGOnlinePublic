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

#include "VirtualProcessProperties.h"
#include "TypeInfoUtils.h"

class IManagedVirtualProcess;
class CReadOnlyMailbox;
class CVirtualProcessMailbox;
class CGetMailboxByIDRequest;
class CGetMailboxByPropertiesRequest;
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

struct STickTime;

enum ETimeType;
enum EConcurrencyManagerState;

namespace tbb
{
	class task_scheduler_init;
}

namespace EVirtualProcessID
{
	enum Enum;
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
		shared_ptr< CVirtualProcessRecord > Get_Record( EVirtualProcessID::Enum process_id ) const;

		shared_ptr< IManagedVirtualProcess > Get_Virtual_Process( EVirtualProcessID::Enum process_id ) const;

		shared_ptr< CWriteOnlyMailbox > Get_Mailbox( EVirtualProcessID::Enum process_id ) const;
		shared_ptr< CReadOnlyMailbox > Get_My_Mailbox( void ) const;

		shared_ptr< CTaskScheduler > Get_Task_Scheduler( ETimeType time_type ) const;

		// Message handling
		void Register_Message_Handlers( void );
		void Register_Handler( const std::type_info &message_type_info, const shared_ptr< IVirtualProcessMessageHandler > &handler );

		void Handle_Message( EVirtualProcessID::Enum source_process_id, const shared_ptr< const IVirtualProcessMessage > &message );
		void Handle_Get_Mailbox_By_ID_Request( EVirtualProcessID::Enum source_process_id, const shared_ptr< const CGetMailboxByIDRequest > &message );
		void Handle_Get_Mailbox_By_Properties_Request( EVirtualProcessID::Enum source_process_id, const shared_ptr< const CGetMailboxByPropertiesRequest > &message );
		void Handle_Add_New_Virtual_Process_Message( EVirtualProcessID::Enum source_process_id, const shared_ptr< const CAddNewVirtualProcessMessage > &message );
		void Handle_Shutdown_Virtual_Process_Message( EVirtualProcessID::Enum source_process_id, const shared_ptr< const CShutdownVirtualProcessMessage > &message );
		void Handle_Reschedule_Virtual_Process_Message( EVirtualProcessID::Enum source_process_id, const shared_ptr< const CRescheduleVirtualProcessMessage > &message );
		void Handle_Release_Mailbox_Response( EVirtualProcessID::Enum source_process_id, const shared_ptr< const CReleaseMailboxResponse > &message );
		void Handle_Shutdown_Self_Response( EVirtualProcessID::Enum source_process_id, const shared_ptr< const CShutdownSelfResponse > &message );
		void Handle_Shutdown_Manager_Message( EVirtualProcessID::Enum source_process_id, const shared_ptr< const CShutdownManagerMessage > &message );

		// Execution
		void Service( void );
		void Service_One_Iteration( void );
		void Service_Shutdown( void );
		void Service_Incoming_Frames( void );

		void Setup_For_Run( const shared_ptr< IManagedVirtualProcess > &starting_process );
		void Shutdown( void );

		// Execution helpers
		void Execute_Virtual_Process( EVirtualProcessID::Enum process_id, double current_time_seconds );

		void Add_Virtual_Process( const shared_ptr< IManagedVirtualProcess > &process );
		void Add_Virtual_Process( const shared_ptr< IManagedVirtualProcess > &process, EVirtualProcessID::Enum id );

		void Send_Virtual_Process_Message( EVirtualProcessID::Enum dest_process_id, const shared_ptr< const IVirtualProcessMessage > &message );
		void Flush_Frames( void );

		void Handle_Ongoing_Mailbox_Requests( CVirtualProcessMailbox *mailbox );
		void Clear_Related_Mailbox_Requests( EVirtualProcessID::Enum process_id );

		// Shutdown helpers
		void Initiate_Process_Shutdown( EVirtualProcessID::Enum process_id );
		bool Is_Process_Shutting_Down( EVirtualProcessID::Enum process_id ) const;
		bool Is_Manager_Shutting_Down( void ) const;
		
		// Time management
		double Get_Game_Time( void ) const;
		void Set_Game_Time( double game_time_seconds );

		// Misc
		EVirtualProcessID::Enum Allocate_Virtual_Process_ID( void );

		// Types
		typedef std::multimap< EVirtualProcessID::Enum, shared_ptr< const CGetMailboxByPropertiesRequest > > GetMailboxByPropertiesRequestCollectionType;

		typedef stdext::hash_map< EVirtualProcessID::Enum, shared_ptr< CVirtualProcessMessageFrame > > FrameTableType;
		typedef stdext::hash_map< Loki::TypeInfo, shared_ptr< IVirtualProcessMessageHandler >, STypeInfoContainerHelper > VirtualProcessMessageHandlerTableType;
		typedef stdext::hash_map< EVirtualProcessID::Enum, shared_ptr< CVirtualProcessRecord > > VirtualProcessRecordTableType;

		typedef stdext::hash_map< EVirtualProcessID::Enum, SProcessProperties > IDToProcessPropertiesTableType;
		typedef std::multimap< SProcessProperties, EVirtualProcessID::Enum, SProcessPropertiesContainerHelper > ProcessPropertiesToIDTableType;

		// Private Data
		VirtualProcessRecordTableType ProcessRecords;
		IDToProcessPropertiesTableType IDToPropertiesTable;
		ProcessPropertiesToIDTableType PropertiesToIDTable;

		GetMailboxByPropertiesRequestCollectionType		PersistentGetRequests;

		FrameTableType PendingOutboundFrames;

		VirtualProcessMessageHandlerTableType MessageHandlers;

		stdext::hash_map< ETimeType, shared_ptr< CTaskScheduler > > TaskSchedulers;
		scoped_ptr< CTimeKeeper > TimeKeeper;

		scoped_ptr< tbb::task_scheduler_init > TBBTaskSchedulerInit;

		EConcurrencyManagerState State;

		EVirtualProcessID::Enum NextID;
};



#endif // CONCURRENCY_MANAGER_H
