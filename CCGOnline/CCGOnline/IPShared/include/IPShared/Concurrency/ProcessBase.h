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

#include <IPShared/IPShared.h>

#include <IPCore/Memory/Stl/Set.h>
#include <IPCore/Memory/Stl/String.h>
#include <IPCore/Memory/Stl/UnorderedMap.h>
#include <IPCore/Memory/Stl/Vector.h>
#include <IPShared/Concurrency/ManagedProcessInterface.h>
#include <IPShared/Concurrency/ProcessProperties.h>
#include <IPShared/TypeInfoUtils.h>

class CProcessBaseTester;
class CProcessBaseExaminer;
class CTaskProcessBaseTester;
class CTaskProcessBaseExaminer;

namespace IP
{
namespace Execution
{
namespace Messaging
{

class CAddMailboxMessage;
class CReleaseMailboxRequest;
class CShutdownSelfRequest;
class IProcessMessageHandler;

} // namespace Messaging

enum EProcessState;

class CProcessMessageFrame;

// The shared logic level of all virtual processes; not instantiable
IPSHARED_API class CProcessBase : public IManagedProcess
{
	public:
		
		using BASECLASS = IManagedProcess;

		// Construction/destruction
		CProcessBase( const SProcessProperties &properties );
		virtual ~CProcessBase();

		// IThreadTask interface
		virtual void Initialize( EProcessID id ) override;

		virtual const SProcessProperties &Get_Properties( void ) const override { return Properties; }
		virtual EProcessID Get_ID( void ) const override { return ID; }

		virtual void Send_Process_Message( EProcessID dest_process_id, IP::UniquePtr< Messaging::IProcessMessage > &message ) override;
		virtual void Send_Process_Message( EProcessID dest_process_id, IP::UniquePtr< Messaging::IProcessMessage > &&message ) override;
		virtual void Send_Manager_Message( IP::UniquePtr< Messaging::IProcessMessage > &message ) override;
		virtual void Send_Manager_Message( IP::UniquePtr< Messaging::IProcessMessage > &&message ) override;
		virtual void Log( IP::String &&message ) override;

		virtual CTaskScheduler *Get_Task_Scheduler( void ) const override { return TaskScheduler.get(); }

		virtual void Flush_System_Messages( void ) override;

		// IManagerThreadTask interface
		virtual void Set_Manager_Mailbox( const std::shared_ptr< CWriteOnlyMailbox > &mailbox ) override;
		virtual void Set_Logging_Mailbox( const std::shared_ptr< CWriteOnlyMailbox > &mailbox ) override;
		virtual void Set_My_Mailbox( const std::shared_ptr< CReadOnlyMailbox > &mailbox ) override;

		virtual void Cleanup( void ) override;

		virtual void Run( const CProcessExecutionContext &context ) override;

		void Register_Handler( const std::type_info &message_type_info, IP::UniquePtr< Messaging::IProcessMessageHandler > &handler );

	protected:

		virtual void Per_Frame_Logic_Start( void ) {}
		virtual void Per_Frame_Logic_End( void ) {}

		bool Is_Shutting_Down( void ) const;

		virtual double Get_Current_Process_Time( void ) const = 0;
		double Get_Next_Task_Time( void ) const;

		virtual void Service_Reschedule( void ) {}

		bool Should_Reschedule( void ) const;

		// protected message handling (derived overridable/modifiable)
		virtual void Register_Message_Handlers( void );

		virtual void On_Shutdown_Self_Request( void ) {}

	private:

		friend class CProcessBaseTester;
		friend class CProcessBaseExaminer;
		friend class CTaskProcessBaseTester;
		friend class CTaskProcessBaseExaminer;

		// private accessors
		std::shared_ptr< CWriteOnlyMailbox > Get_Mailbox( EProcessID process_id ) const;

		// Private message handling
		void Flush_Regular_Messages( void );

		void Service_Message_Frames( void );
		void Handle_Message( EProcessID process_id, IP::UniquePtr< const Messaging::IProcessMessage > &message );

		void Handle_Add_Mailbox_Message( EProcessID process_id, IP::UniquePtr< const Messaging::CAddMailboxMessage > &message );
		void Handle_Release_Mailbox_Request( EProcessID process_id, IP::UniquePtr< const Messaging::CReleaseMailboxRequest > &request );
		void Handle_Shutdown_Self_Request( EProcessID process_id, IP::UniquePtr< const Messaging::CShutdownSelfRequest > &message );

		void Handle_Shutdown_Mailboxes( void );

		void Build_Process_ID_List_By_Properties( const SProcessProperties &properties, IP::Vector< EProcessID > &process_ids ) const;
		void Remove_Process_ID_From_Tables( EProcessID process_id );

		// Type definitions
		using MailboxTableType = IP::UnorderedMap< EProcessID, std::shared_ptr< CWriteOnlyMailbox > >;
		using IDToProcessPropertiesTableType = IP::UnorderedMap< EProcessID, SProcessProperties >;
		using ProcessPropertiesToIDTableType = IP::UnorderedMultiMap< SProcessProperties, EProcessID, SProcessPropertiesContainerHelper >;
		using ProcessMessageHandlerTableType = IP::UnorderedMap< Loki::TypeInfo, IP::UniquePtr<  Messaging::IProcessMessageHandler >, STypeInfoContainerHelper >;
		using FrameTableType = IP::UnorderedMap< EProcessID, IP::UniquePtr< CProcessMessageFrame > >;

		// Private Data
		// Simple state
		EProcessID ID;
		SProcessProperties Properties;

		EProcessState State;

		// Message frames
		FrameTableType PendingOutboundFrames;
		IP::UniquePtr< CProcessMessageFrame > ManagerFrame;
		IP::UniquePtr< CProcessMessageFrame > LogFrame;

		// Process interfaces
		IDToProcessPropertiesTableType IDToPropertiesTable;
		ProcessPropertiesToIDTableType PropertiesToIDTable;

		MailboxTableType Mailboxes;
		std::shared_ptr< CWriteOnlyMailbox > ManagerMailbox;
		std::shared_ptr< CWriteOnlyMailbox > LoggingMailbox;

		std::shared_ptr< CReadOnlyMailbox > MyMailbox;

		IP::Set< EProcessID > ShutdownMailboxes;

		// Timing
		double FirstServiceTimeSeconds;
		double CurrentTimeSeconds;

		// Misc
		ProcessMessageHandlerTableType MessageHandlers;

		IP::UniquePtr< CTaskScheduler > TaskScheduler;
};

} // namespace Execution
} // namespace IP
