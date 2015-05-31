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

#ifndef PROCESS_BASE_H
#define PROCESS_BASE_H

#include "ManagedProcessInterface.h"

#include "ProcessProperties.h"

class CAddMailboxMessage;
class CReleaseMailboxRequest;
class CShutdownSelfRequest;
class IProcessMessageHandler;
class CProcessMessageFrame;

struct STickTime;

enum EProcessState;

// The shared logic level of all virtual processes; not instantiable
class CProcessBase : public IManagedProcess
{
	public:
		
		typedef IManagedProcess BASECLASS;

		// Construction/destruction
		CProcessBase( const SProcessProperties &properties );
		virtual ~CProcessBase();

		// IThreadTask interface
		virtual void Initialize( EProcessID::Enum id ) override;

		virtual const SProcessProperties &Get_Properties( void ) const override { return Properties; }
		virtual EProcessID::Enum Get_ID( void ) const override { return ID; }

		virtual void Send_Process_Message( EProcessID::Enum dest_process_id, unique_ptr< const IProcessMessage > &message ) override;
		virtual void Send_Process_Message( EProcessID::Enum dest_process_id, unique_ptr< const IProcessMessage > &&message ) override;
		virtual void Send_Manager_Message( unique_ptr< const IProcessMessage > &message ) override;
		virtual void Send_Manager_Message( unique_ptr< const IProcessMessage > &&message ) override;
		virtual void Log( std::wstring &&message ) override;

		virtual CTaskScheduler *Get_Task_Scheduler( void ) const override { return TaskScheduler.get(); }

		virtual void Flush_System_Messages( void ) override;

		// IManagerThreadTask interface
		virtual void Set_Manager_Mailbox( const shared_ptr< CWriteOnlyMailbox > &mailbox ) override;
		virtual void Set_Logging_Mailbox( const shared_ptr< CWriteOnlyMailbox > &mailbox ) override;
		virtual void Set_My_Mailbox( const shared_ptr< CReadOnlyMailbox > &mailbox ) override;

		virtual void Cleanup( void ) override;

		virtual void Run( const CProcessExecutionContext &context ) override;

		void Register_Handler( const std::type_info &message_type_info, unique_ptr< IProcessMessageHandler > &handler );

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
		shared_ptr< CWriteOnlyMailbox > Get_Mailbox( EProcessID::Enum process_id ) const;

		// Private message handling
		void Flush_Regular_Messages( void );

		void Service_Message_Frames( void );
		void Handle_Message( EProcessID::Enum process_id, unique_ptr< const IProcessMessage > &message );

		void Handle_Add_Mailbox_Message( EProcessID::Enum process_id, unique_ptr< const CAddMailboxMessage > &message );
		void Handle_Release_Mailbox_Request( EProcessID::Enum process_id, unique_ptr< const CReleaseMailboxRequest > &request );
		void Handle_Shutdown_Self_Request( EProcessID::Enum process_id, unique_ptr< const CShutdownSelfRequest > &message );

		void Handle_Shutdown_Mailboxes( void );

		void Build_Process_ID_List_By_Properties( const SProcessProperties &properties, std::vector< EProcessID::Enum > &process_ids ) const;
		void Remove_Process_ID_From_Tables( EProcessID::Enum process_id );

		// Type definitions
		typedef std::unordered_map< EProcessID::Enum, shared_ptr< CWriteOnlyMailbox > > MailboxTableType;
		typedef std::unordered_map< EProcessID::Enum, SProcessProperties > IDToProcessPropertiesTableType;
		typedef std::unordered_multimap< SProcessProperties, EProcessID::Enum, SProcessPropertiesContainerHelper > ProcessPropertiesToIDTableType;
		typedef std::unordered_map< Loki::TypeInfo, unique_ptr< IProcessMessageHandler >, STypeInfoContainerHelper > ProcessMessageHandlerTableType;
		typedef std::unordered_map< EProcessID::Enum, unique_ptr< CProcessMessageFrame > > FrameTableType;

		// Private Data
		// Simple state
		EProcessID::Enum ID;
		SProcessProperties Properties;

		EProcessState State;

		// Message frames
		FrameTableType PendingOutboundFrames;
		unique_ptr< CProcessMessageFrame > ManagerFrame;
		unique_ptr< CProcessMessageFrame > LogFrame;

		// Process interfaces
		IDToProcessPropertiesTableType IDToPropertiesTable;
		ProcessPropertiesToIDTableType PropertiesToIDTable;

		MailboxTableType Mailboxes;
		shared_ptr< CWriteOnlyMailbox > ManagerMailbox;
		shared_ptr< CWriteOnlyMailbox > LoggingMailbox;

		shared_ptr< CReadOnlyMailbox > MyMailbox;

		std::set< EProcessID::Enum > ShutdownMailboxes;

		// Timing
		double FirstServiceTimeSeconds;
		double CurrentTimeSeconds;

		// Misc
		ProcessMessageHandlerTableType MessageHandlers;

		unique_ptr< CTaskScheduler > TaskScheduler;
};

#endif // PROCESS_BASE_H
