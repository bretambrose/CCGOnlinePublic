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
#include "Concurrency/ThreadKey.h"

class IVirtualProcess;

// Thread -> Manager
// Requests that a new thread be added to the concurrency system
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

// Thread -> Manager
// Requests that a thread be rescheduled for execution
class CRescheduleVirtualProcessMessage : public IVirtualProcessMessage
{
	public:

		typedef IVirtualProcessMessage BASECLASS;
		
		CRescheduleVirtualProcessMessage( const SThreadKey &key, double reschedule_time ) :
			Key( key ),
			RescheduleTime( reschedule_time )
		{}

		virtual ~CRescheduleVirtualProcessMessage() {}

		const SThreadKey &Get_Key( void ) const { return Key; }
		double Get_Reschedule_Time( void ) const { return RescheduleTime; }

	private:

		SThreadKey Key;

		double RescheduleTime;
};

// Manager -> thread
// Tells a thread that another thread is going away soon, so stop using that thread's mailbox
class CReleaseMailboxRequest : public IVirtualProcessMessage
{
	public:
		
		typedef IVirtualProcessMessage BASECLASS;

		CReleaseMailboxRequest( const SThreadKey &key ) :
			Key( key )
		{}

		virtual ~CReleaseMailboxRequest() {}

		const SThreadKey &Get_Key( void ) const { return Key; }

	private:

		SThreadKey Key;
};

// Thread -> Manager
// Tells the manager that a thread has stopped using another virtual process's mailbox
class CReleaseMailboxResponse : public IVirtualProcessMessage
{
	public:
		
		typedef IVirtualProcessMessage BASECLASS;

		CReleaseMailboxResponse( const SThreadKey &shutdown_key ) :
			ShutdownKey( shutdown_key )
		{}

		virtual ~CReleaseMailboxResponse() {}

		const SThreadKey &Get_Shutdown_Key( void ) const { return ShutdownKey; }

	private:

		SThreadKey ShutdownKey;
};

// Thread -> manager
// Requests that the manager shut down another thread
class CShutdownVirtualProcessMessage : public IVirtualProcessMessage
{
	public:

		typedef IVirtualProcessMessage BASECLASS;
		
		CShutdownVirtualProcessMessage( const SThreadKey &key ) :
			Key( key )
		{}

		virtual ~CShutdownVirtualProcessMessage() {}

		const SThreadKey &Get_Key( void ) const { return Key; }

	private:

		SThreadKey Key;
};

// Manager -> thread
// Tells a thread that it should shut down and clean up any internal state
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

// thread -> manager
// Tells the manager that the sending thread has successfully shut down
class CShutdownSelfResponse : public IVirtualProcessMessage
{
	public:
		
		typedef IVirtualProcessMessage BASECLASS;

		CShutdownSelfResponse( void ) {}

		virtual ~CShutdownSelfResponse() {}

	private:

};

// Thread -> Manager
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