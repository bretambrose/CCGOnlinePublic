/**********************************************************************************************************************

	ProcessManagementMessages.h
		A component containing definitions for process messages that manage and/or control processes

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

#ifndef PROCESS_MANAGEMENT_MESSAGES_H
#define PROCESS_MANAGEMENT_MESSAGES_H

#include "ProcessMessage.h"
#include "IPShared/Concurrency/ProcessProperties.h"

class IProcess;

namespace EProcessID
{
	enum Enum;
}

// Process -> Manager
// Requests that a new process be added to the concurrency system
class CAddNewProcessMessage : public IProcessMessage
{
	public:

		typedef IProcessMessage BASECLASS;
		
		CAddNewProcessMessage( const shared_ptr< IProcess > &process, bool return_mailbox, bool forward_creator_mailbox );
		virtual ~CAddNewProcessMessage();

		shared_ptr< IProcess > Get_Process( void ) const { return Process; }
		bool Should_Return_Mailbox( void ) const { return ReturnMailbox; }
		bool Should_Forward_Creator_Mailbox( void ) const { return ForwardCreatorMailbox; }

	private:

		shared_ptr< IProcess > Process;

		bool ReturnMailbox;
		bool ForwardCreatorMailbox;
};

// Process -> Manager
// Requests that a process be rescheduled for execution
class CRescheduleProcessMessage : public IProcessMessage
{
	public:

		typedef IProcessMessage BASECLASS;
		
		CRescheduleProcessMessage( double reschedule_time ) :
			RescheduleTime( reschedule_time )
		{}

		virtual ~CRescheduleProcessMessage() {}

		double Get_Reschedule_Time( void ) const { return RescheduleTime; }

	private:

		double RescheduleTime;
};

// Manager -> process
// Tells a process that another process is going away soon, so stop using that process's mailbox
class CReleaseMailboxRequest : public IProcessMessage
{
	public:
		
		typedef IProcessMessage BASECLASS;

		CReleaseMailboxRequest( EProcessID::Enum process_id ) :
			ProcessID( process_id )
		{}

		virtual ~CReleaseMailboxRequest() {}

		EProcessID::Enum Get_Process_ID( void ) const { return ProcessID; }

	private:

		EProcessID::Enum ProcessID;
};

// Process -> Manager
// Tells the manager that a process has stopped using another virtual process's mailbox
class CReleaseMailboxResponse : public IProcessMessage
{
	public:
		
		typedef IProcessMessage BASECLASS;

		CReleaseMailboxResponse( EProcessID::Enum shutdown_process_id ) :
			ShutdownProcessID( shutdown_process_id )
		{}

		virtual ~CReleaseMailboxResponse() {}

		EProcessID::Enum Get_Shutdown_Process_ID( void ) const { return ShutdownProcessID; }

	private:

		EProcessID::Enum ShutdownProcessID;
};

// Process -> manager
// Requests that the manager shut down another process
class CShutdownProcessMessage : public IProcessMessage
{
	public:

		typedef IProcessMessage BASECLASS;
		
		CShutdownProcessMessage( EProcessID::Enum process_id ) :
			ProcessID( process_id )
		{}

		virtual ~CShutdownProcessMessage() {}

		EProcessID::Enum Get_Process_ID( void ) const { return ProcessID; }

	private:

		EProcessID::Enum ProcessID;
};

// Manager -> process
// Tells a process that it should shut down and clean up any internal state
class CShutdownSelfRequest : public IProcessMessage
{
	public:
		
		typedef IProcessMessage BASECLASS;

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
class CShutdownSelfResponse : public IProcessMessage
{
	public:
		
		typedef IProcessMessage BASECLASS;

		CShutdownSelfResponse( void ) {}

		virtual ~CShutdownSelfResponse() {}

	private:

};

// Process -> Manager
// Requests the entire concurrency system to be shut down
class CShutdownManagerMessage : public IProcessMessage
{
	public:

		typedef IProcessMessage BASECLASS;
		
		CShutdownManagerMessage( void ) {}

		virtual ~CShutdownManagerMessage() {}

	private:

};

#endif // PROCESS_MANAGEMENT_MESSAGES_H