/**********************************************************************************************************************

	VirtualProcessBase.h
		A component containing the logic shared by all virtual processes.

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

#ifndef VIRTUAL_PROCESS_BASE_H
#define VIRTUAL_PROCESS_BASE_H

#include "ManagedVirtualProcessInterface.h"

#include "VirtualProcessProperties.h"
#include "TypeInfoUtils.h"

class CAddMailboxMessage;
class CReleaseMailboxRequest;
class CShutdownSelfRequest;
class IVirtualProcessMessageHandler;
class CVirtualProcessMessageFrame;

struct STickTime;

enum EVirtualProcessState;

// The shared logic level of all task-based virtual processes; not instantiable
class CVirtualProcessBase : public IManagedVirtualProcess
{
	public:
		
		typedef IManagedVirtualProcess BASECLASS;

		// Construction/destruction
		CVirtualProcessBase( const SProcessProperties &properties );
		virtual ~CVirtualProcessBase();

		// IThreadTask interface
		virtual void Initialize( EVirtualProcessID::Enum id );

		virtual const SProcessProperties &Get_Properties( void ) const { return Properties; }
		virtual EVirtualProcessID::Enum Get_ID( void ) const { return ID; }

		virtual void Send_Virtual_Process_Message( EVirtualProcessID::Enum dest_process_id, const shared_ptr< const IVirtualProcessMessage > &message );
		virtual void Log( const std::wstring &message );

		virtual CTaskScheduler *Get_Task_Scheduler( void ) const { return TaskScheduler.get(); }

		virtual void Flush_System_Messages( void );

		// IManagerThreadTask interface
		virtual void Set_Manager_Mailbox( const shared_ptr< CWriteOnlyMailbox > &mailbox );
		virtual void Set_Logging_Mailbox( const shared_ptr< CWriteOnlyMailbox > &mailbox );
		virtual void Set_My_Mailbox( const shared_ptr< CReadOnlyMailbox > &mailbox );

		virtual void Cleanup( void );

		virtual void Service( double elapsed_seconds, const CVirtualProcessExecutionContext &context );

		virtual double Get_Elapsed_Seconds( void ) const;

	protected:

		double Get_Current_Thread_Time( void ) const;

		// Scheduling interface, intended for derived classes
		virtual double Get_Reschedule_Time( void ) const;
		virtual double Get_Reschedule_Interval( void ) const;
		virtual bool Should_Reschedule( void ) const;

		// protected message handling (derived overridable/modifiable)
		virtual void Register_Message_Handlers( void );

		void Register_Handler( const std::type_info &message_type_info, const shared_ptr< IVirtualProcessMessageHandler > &handler );

		virtual void Handle_Shutdown_Self_Request( EVirtualProcessID::Enum process_id, const shared_ptr< const CShutdownSelfRequest > &message );

	private:

		friend class CVirtualProcessBaseTester;
		friend class CVirtualProcessBaseExaminer;

		// private accessors
		shared_ptr< CWriteOnlyMailbox > Get_Mailbox( EVirtualProcessID::Enum process_id ) const;
		bool Is_Shutting_Down( void ) const;

		// Private message handling
		void Flush_Regular_Messages( void );

		void Service_Message_Frames( void );
		void Handle_Message( EVirtualProcessID::Enum processS_id, const shared_ptr< const IVirtualProcessMessage > &message );

		void Handle_Add_Mailbox_Message( EVirtualProcessID::Enum process_id, const shared_ptr< const CAddMailboxMessage > &message );
		void Handle_Release_Mailbox_Request( EVirtualProcessID::Enum process_id, const shared_ptr< const CReleaseMailboxRequest > &request );

		void Handle_Shutdown_Mailboxes( void );

		void Build_Process_ID_List_By_Properties( const SProcessProperties &properties, std::vector< EVirtualProcessID::Enum > &process_ids ) const;
		void Remove_Process_ID_From_Tables( EVirtualProcessID::Enum process_id );

		// Type definitions
		typedef stdext::hash_map< EVirtualProcessID::Enum, shared_ptr< CWriteOnlyMailbox > > MailboxTableType;
		typedef stdext::hash_map< EVirtualProcessID::Enum, shared_ptr< CWriteOnlyMailbox > > MailboxTableType;
		typedef stdext::hash_map< EVirtualProcessID::Enum, shared_ptr< CVirtualProcessMessageFrame > > FrameTableType;
		typedef stdext::hash_map< EVirtualProcessID::Enum, SProcessProperties > IDToProcessPropertiesTableType;
		typedef std::multimap< SProcessProperties, EVirtualProcessID::Enum, SProcessPropertiesContainerHelper > ProcessPropertiesToIDTableType;
		typedef stdext::hash_map< Loki::TypeInfo, shared_ptr< IVirtualProcessMessageHandler >, STypeInfoContainerHelper > VirtualProcessMessageHandlerTableType;

		// Private Data
		// Simple state
		EVirtualProcessID::Enum ID;
		SProcessProperties Properties;

		EVirtualProcessState State;

		// Message frames
		FrameTableType PendingOutboundFrames;
		shared_ptr< CVirtualProcessMessageFrame > ManagerFrame;
		shared_ptr< CVirtualProcessMessageFrame > LogFrame;

		// Process interfaces
		IDToProcessPropertiesTableType IDToPropertiesTable;
		ProcessPropertiesToIDTableType PropertiesToIDTable;

		MailboxTableType Mailboxes;
		shared_ptr< CWriteOnlyMailbox > ManagerMailbox;
		shared_ptr< CWriteOnlyMailbox > LoggingMailbox;

		shared_ptr< CReadOnlyMailbox > MyMailbox;

		std::set< EVirtualProcessID::Enum > ShutdownMailboxes;

		// Timing
		double FirstServiceTimeSeconds;
		double CurrentTimeSeconds;

		// Misc
		VirtualProcessMessageHandlerTableType MessageHandlers;

		scoped_ptr< CTaskScheduler > TaskScheduler;
};

#endif // VIRTUAL_PROCESS_BASE_H
