/**********************************************************************************************************************

	ConcurrencyManager.h
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

#ifndef CONCURRENCY_MANAGER_H
#define CONCURRENCY_MANAGER_H

#include "ProcessProperties.h"
#include "TypeInfoUtils.h"

class IManagedProcess;
class CReadOnlyMailbox;
class CProcessMailbox;
class CGetMailboxByIDRequest;
class CGetMailboxByPropertiesRequest;
class CAddNewProcessMessage;
class CShutdownProcessMessage;
class CRescheduleProcessMessage;
class CReleaseMailboxResponse;
class CShutdownSelfResponse;
class CShutdownManagerMessage;
class CWriteOnlyMailbox;
class IProcessMessage;
class IProcessMessageHandler;
class CProcessMessageFrame;
class CTaskScheduler;
class CTimeKeeper;
class CProcessRecord;

struct STickTime;

enum ETimeType;
enum EConcurrencyManagerState;

namespace tbb
{
	class task_scheduler_init;
}

namespace EProcessID
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

		void Run( const shared_ptr< IManagedProcess > &starting_process );

		void Log( const std::wstring &message );

	private:

		friend class CConcurrencyManagerTester;

		// Accessors
		shared_ptr< CProcessRecord > Get_Record( EProcessID::Enum process_id ) const;

		shared_ptr< IManagedProcess > Get_Process( EProcessID::Enum process_id ) const;
		void Enumerate_Processes( std::vector< shared_ptr< IManagedProcess > > &processes ) const;

		shared_ptr< CWriteOnlyMailbox > Get_Mailbox( EProcessID::Enum process_id ) const;
		shared_ptr< CReadOnlyMailbox > Get_My_Mailbox( void ) const;

		shared_ptr< CTaskScheduler > Get_Task_Scheduler( ETimeType time_type ) const;

		// Message handling
		void Register_Message_Handlers( void );
		void Register_Handler( const std::type_info &message_type_info, const shared_ptr< IProcessMessageHandler > &handler );

		void Handle_Message( EProcessID::Enum source_process_id, const shared_ptr< const IProcessMessage > &message );
		void Handle_Get_Mailbox_By_ID_Request( EProcessID::Enum source_process_id, const shared_ptr< const CGetMailboxByIDRequest > &message );
		void Handle_Get_Mailbox_By_Properties_Request( EProcessID::Enum source_process_id, const shared_ptr< const CGetMailboxByPropertiesRequest > &message );
		void Handle_Add_New_Process_Message( EProcessID::Enum source_process_id, const shared_ptr< const CAddNewProcessMessage > &message );
		void Handle_Shutdown_Process_Message( EProcessID::Enum source_process_id, const shared_ptr< const CShutdownProcessMessage > &message );
		void Handle_Reschedule_Process_Message( EProcessID::Enum source_process_id, const shared_ptr< const CRescheduleProcessMessage > &message );
		void Handle_Release_Mailbox_Response( EProcessID::Enum source_process_id, const shared_ptr< const CReleaseMailboxResponse > &message );
		void Handle_Shutdown_Self_Response( EProcessID::Enum source_process_id, const shared_ptr< const CShutdownSelfResponse > &message );
		void Handle_Shutdown_Manager_Message( EProcessID::Enum source_process_id, const shared_ptr< const CShutdownManagerMessage > &message );

		// Execution
		void Service( void );
		void Service_One_Iteration( void );
		void Service_Shutdown( void );
		void Service_Incoming_Frames( void );

		void Setup_For_Run( const shared_ptr< IManagedProcess > &starting_process );
		void Shutdown( void );

		// Execution helpers
		void Execute_Process( EProcessID::Enum process_id, double current_time_seconds );

		void Add_Process( const shared_ptr< IManagedProcess > &process );
		void Add_Process( const shared_ptr< IManagedProcess > &process, EProcessID::Enum id );

		void Send_Process_Message( EProcessID::Enum dest_process_id, const shared_ptr< const IProcessMessage > &message );
		void Flush_Frames( void );

		void Handle_Ongoing_Mailbox_Requests( CProcessMailbox *mailbox );
		void Clear_Related_Mailbox_Requests( EProcessID::Enum process_id );

		// Shutdown helpers
		void Initiate_Process_Shutdown( EProcessID::Enum process_id );
		bool Is_Process_Shutting_Down( EProcessID::Enum process_id ) const;
		bool Is_Manager_Shutting_Down( void ) const;
		
		// Time management
		double Get_Game_Time( void ) const;
		void Set_Game_Time( double game_time_seconds );

		// Misc
		EProcessID::Enum Allocate_Process_ID( void );

		// Types
		typedef std::multimap< EProcessID::Enum, shared_ptr< const CGetMailboxByPropertiesRequest > > GetMailboxByPropertiesRequestCollectionType;

		typedef stdext::hash_map< EProcessID::Enum, shared_ptr< CProcessMessageFrame > > FrameTableType;
		typedef stdext::hash_map< Loki::TypeInfo, shared_ptr< IProcessMessageHandler >, STypeInfoContainerHelper > ProcessMessageHandlerTableType;
		typedef stdext::hash_map< EProcessID::Enum, shared_ptr< CProcessRecord > > ProcessRecordTableType;

		typedef stdext::hash_map< EProcessID::Enum, SProcessProperties > IDToProcessPropertiesTableType;
		typedef std::multimap< SProcessProperties, EProcessID::Enum, SProcessPropertiesContainerHelper > ProcessPropertiesToIDTableType;

		// Private Data
		ProcessRecordTableType ProcessRecords;
		IDToProcessPropertiesTableType IDToPropertiesTable;
		ProcessPropertiesToIDTableType PropertiesToIDTable;

		GetMailboxByPropertiesRequestCollectionType		PersistentGetRequests;

		FrameTableType PendingOutboundFrames;

		ProcessMessageHandlerTableType MessageHandlers;

		stdext::hash_map< ETimeType, shared_ptr< CTaskScheduler > > TaskSchedulers;
		unique_ptr< CTimeKeeper > TimeKeeper;

		unique_ptr< tbb::task_scheduler_init > TBBTaskSchedulerInit;

		EConcurrencyManagerState State;

		EProcessID::Enum NextID;
};



#endif // CONCURRENCY_MANAGER_H
