/**********************************************************************************************************************

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

#pragma once

#include "ProcessProperties.h"

namespace tbb
{
	class task_scheduler_init;
}

class CConcurrencyManagerTester;

namespace IP
{
namespace Time
{

class CTimeKeeper;

} // namespace Time

namespace Execution
{
namespace Messaging
{

class IProcessMessage;
class IProcessMessageHandler;
class CAddNewProcessMessage;
class CShutdownProcessMessage;
class CRescheduleProcessMessage;
class CReleaseMailboxResponse;
class CShutdownSelfResponse;
class CShutdownManagerMessage;
class CGetMailboxByIDRequest;
class CGetMailboxByPropertiesRequest;

} // namespace Messaging

class IManagedProcess;
class CReadOnlyMailbox;
class CProcessMailbox;
class CWriteOnlyMailbox;
class CProcessMessageFrame;
class CTaskScheduler;
class CProcessRecord;

enum class EConcurrencyManagerState;
enum class EProcessID;

// the central, manager class that manages all thread tasks
class CConcurrencyManager
{
	public:

		// Construction/Destruction
		CConcurrencyManager( void );
		~CConcurrencyManager();

		// Public interface
		void Initialize( bool delete_all_logs );

		void Run( const std::shared_ptr< IManagedProcess > &starting_process );

		void Log( std::wstring &&message );

		void Register_Handler( const std::type_info &message_type_info, std::unique_ptr< Messaging::IProcessMessageHandler > &handler );

	private:

		friend class CConcurrencyManagerTester;

		// Accessors
		std::shared_ptr< CProcessRecord > Get_Record( EProcessID process_id ) const;

		std::shared_ptr< IManagedProcess > Get_Process( EProcessID process_id ) const;
		void Enumerate_Processes( std::vector< std::shared_ptr< IManagedProcess > > &processes ) const;

		std::shared_ptr< CWriteOnlyMailbox > Get_Mailbox( EProcessID process_id ) const;
		const std::shared_ptr< CReadOnlyMailbox >& Get_My_Mailbox( void ) const;

		const std::shared_ptr< CTaskScheduler >& Get_Task_Scheduler( void ) const;

		// Message handling
		void Register_Message_Handlers( void );

		void Handle_Message( EProcessID source_process_id, std::unique_ptr< const Messaging::IProcessMessage > &message );
		void Handle_Get_Mailbox_By_ID_Request( EProcessID source_process_id, std::unique_ptr< const Messaging::CGetMailboxByIDRequest > &message );
		void Handle_Get_Mailbox_By_Properties_Request( EProcessID source_process_id, std::unique_ptr< const Messaging::CGetMailboxByPropertiesRequest > &message );
		void Handle_Add_New_Process_Message( EProcessID source_process_id, std::unique_ptr< const Messaging::CAddNewProcessMessage > &message );
		void Handle_Shutdown_Process_Message( EProcessID source_process_id, std::unique_ptr< const Messaging::CShutdownProcessMessage > &message );
		void Handle_Reschedule_Process_Message( EProcessID source_process_id, std::unique_ptr< const Messaging::CRescheduleProcessMessage > &message );
		void Handle_Release_Mailbox_Response( EProcessID source_process_id, std::unique_ptr< const Messaging::CReleaseMailboxResponse > &message );
		void Handle_Shutdown_Self_Response( EProcessID source_process_id, std::unique_ptr< const Messaging::CShutdownSelfResponse > &message );
		void Handle_Shutdown_Manager_Message( EProcessID source_process_id, std::unique_ptr< const Messaging::CShutdownManagerMessage > &message );

		// Execution
		void Service( void );
		void Service_One_Iteration( void );
		void Service_Shutdown( void );
		void Service_Incoming_Frames( void );

		void Setup_For_Run( const std::shared_ptr< IManagedProcess > &starting_process );
		void Shutdown( void );

		// Execution helpers
		void Execute_Process( EProcessID process_id, double current_time_seconds );

		void Add_Process( const std::shared_ptr< IManagedProcess > &process );
		void Add_Process( const std::shared_ptr< IManagedProcess > &process, EProcessID id );

		void Send_Process_Message( EProcessID dest_process_id, std::unique_ptr< const Messaging::IProcessMessage > &message );
		void Flush_Frames( void );

		void Handle_Ongoing_Mailbox_Requests( CProcessMailbox *mailbox );
		void Clear_Related_Mailbox_Requests( EProcessID process_id );

		// Shutdown helpers
		void Initiate_Process_Shutdown( EProcessID process_id );
		bool Is_Process_Shutting_Down( EProcessID process_id ) const;
		bool Is_Manager_Shutting_Down( void ) const;

		// Misc
		EProcessID Allocate_Process_ID( void );

		// Types
		using GetMailboxByPropertiesRequestCollectionType = std::multimap< EProcessID, std::unique_ptr< const Messaging::CGetMailboxByPropertiesRequest > >;

		using FrameTableType = std::unordered_map< EProcessID, std::unique_ptr< CProcessMessageFrame > >;
		using ProcessMessageHandlerTableType = std::unordered_map< Loki::TypeInfo, std::unique_ptr< Messaging::IProcessMessageHandler >, STypeInfoContainerHelper >;
		using ProcessRecordTableType = std::unordered_map< EProcessID, std::shared_ptr< CProcessRecord > >;

		using IDToProcessPropertiesTableType = std::unordered_map< EProcessID, SProcessProperties >;
		using ProcessPropertiesToIDTableType = std::unordered_multimap< SProcessProperties, EProcessID, SProcessPropertiesContainerHelper >;

		// Private Data
		ProcessRecordTableType ProcessRecords;
		IDToProcessPropertiesTableType IDToPropertiesTable;
		ProcessPropertiesToIDTableType PropertiesToIDTable;

		GetMailboxByPropertiesRequestCollectionType		PersistentGetRequests;

		FrameTableType PendingOutboundFrames;

		ProcessMessageHandlerTableType MessageHandlers;

		std::shared_ptr< CTaskScheduler > TaskScheduler;
		std::unique_ptr< IP::Time::CTimeKeeper > TimeKeeper;

		std::unique_ptr< tbb::task_scheduler_init > TBBTaskSchedulerInit;

		EConcurrencyManagerState State;

		EProcessID NextID;
};

} // namespace Execution
} // namespace IP
