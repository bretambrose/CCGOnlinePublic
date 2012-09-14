/**********************************************************************************************************************

	ThreadManagementMessages.h
		A component containing definitions for virtual process messages that manage and/or control virtual processes

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

#ifndef VIRTUAL_PROCESS_MANAGEMENT_MESSAGES_H
#define VIRTUAL_PROCESS_MANAGEMENT_MESSAGES_H

#include "VirtualProcessMessage.h"
#include "Concurrency/VirtualProcessProperties.h"

class IVirtualProcess;

namespace EVirtualProcessID
{
	enum Enum;
}

// Process -> Manager
// Requests that a new process be added to the concurrency system
class CAddNewVirtualProcessMessage : public IVirtualProcessMessage
{
	public:

		typedef IVirtualProcessMessage BASECLASS;
		
		CAddNewVirtualProcessMessage( const shared_ptr< IVirtualProcess > &virtual_process, bool return_mailbox, bool forward_creator_mailbox );
		virtual ~CAddNewVirtualProcessMessage();

		shared_ptr< IVirtualProcess > Get_Virtual_Process( void ) const { return VirtualProcess; }
		bool Should_Return_Mailbox( void ) const { return ReturnMailbox; }
		bool Should_Forward_Creator_Mailbox( void ) const { return ForwardCreatorMailbox; }

	private:

		shared_ptr< IVirtualProcess > VirtualProcess;

		bool ReturnMailbox;
		bool ForwardCreatorMailbox;
};

// Process -> Manager
// Requests that a process be rescheduled for execution
class CRescheduleVirtualProcessMessage : public IVirtualProcessMessage
{
	public:

		typedef IVirtualProcessMessage BASECLASS;
		
		CRescheduleVirtualProcessMessage( double reschedule_time ) :
			RescheduleTime( reschedule_time )
		{}

		virtual ~CRescheduleVirtualProcessMessage() {}

		double Get_Reschedule_Time( void ) const { return RescheduleTime; }

	private:

		double RescheduleTime;
};

// Manager -> process
// Tells a process that another process is going away soon, so stop using that process's mailbox
class CReleaseMailboxRequest : public IVirtualProcessMessage
{
	public:
		
		typedef IVirtualProcessMessage BASECLASS;

		CReleaseMailboxRequest( EVirtualProcessID::Enum process_id ) :
			ProcessID( process_id )
		{}

		virtual ~CReleaseMailboxRequest() {}

		EVirtualProcessID::Enum Get_Process_ID( void ) const { return ProcessID; }

	private:

		EVirtualProcessID::Enum ProcessID;
};

// Process -> Manager
// Tells the manager that a process has stopped using another virtual process's mailbox
class CReleaseMailboxResponse : public IVirtualProcessMessage
{
	public:
		
		typedef IVirtualProcessMessage BASECLASS;

		CReleaseMailboxResponse( EVirtualProcessID::Enum shutdown_process_id ) :
			ShutdownProcessID( shutdown_process_id )
		{}

		virtual ~CReleaseMailboxResponse() {}

		EVirtualProcessID::Enum Get_Shutdown_Process_ID( void ) const { return ShutdownProcessID; }

	private:

		EVirtualProcessID::Enum ShutdownProcessID;
};

// Process -> manager
// Requests that the manager shut down another process
class CShutdownVirtualProcessMessage : public IVirtualProcessMessage
{
	public:

		typedef IVirtualProcessMessage BASECLASS;
		
		CShutdownVirtualProcessMessage( EVirtualProcessID::Enum process_id ) :
			ProcessID( process_id )
		{}

		virtual ~CShutdownVirtualProcessMessage() {}

		EVirtualProcessID::Enum Get_Process_ID( void ) const { return ProcessID; }

	private:

		EVirtualProcessID::Enum ProcessID;
};

// Manager -> process
// Tells a process that it should shut down and clean up any internal state
class CShutdownSelfRequest : public IVirtualProcessMessage
{
	public:
		
		typedef IVirtualProcessMessage BASECLASS;

		CShutdownSelfRequest( bool is_hard_shutdown ) :
			IsHardShutdown( is_hard_shutdown )
		{}

		virtual ~CShutdownSelfRequest() {}

		bool Get_Is_Hard_Shutdown( void ) const { return IsHardShutdown; }

	private:

		bool IsHardShutdown;
};

// Process -> manager
// Tells the manager that the sending process has successfully shut down
class CShutdownSelfResponse : public IVirtualProcessMessage
{
	public:
		
		typedef IVirtualProcessMessage BASECLASS;

		CShutdownSelfResponse( void ) {}

		virtual ~CShutdownSelfResponse() {}

	private:

};

// Process -> Manager
// Requests the entire concurrency system to be shut down
class CShutdownManagerMessage : public IVirtualProcessMessage
{
	public:

		typedef IVirtualProcessMessage BASECLASS;
		
		CShutdownManagerMessage( void ) {}

		virtual ~CShutdownManagerMessage() {}

	private:

};

#endif // VIRTUAL_PROCESS_MANAGEMENT_MESSAGES_H