/**********************************************************************************************************************

	ThreadManagementMessages.h
		A component containing definitions for thread messages that manage and/or control thread tasks

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

#ifndef THREAD_MANAGEMENT_MESSAGES_H
#define THREAD_MANAGEMENT_MESSAGES_H

#include "ThreadMessage.h"
#include "Concurrency/ThreadKey.h"

class IThreadTask;

// Thread -> Manager
// Requests that a new thread be added to the concurrency system
class CAddThreadMessage : public IThreadMessage
{
	public:

		typedef IThreadMessage BASECLASS;
		
		CAddThreadMessage( const shared_ptr< IThreadTask > &thread_task, bool return_interface, bool forward_creator_interface );
		virtual ~CAddThreadMessage();

		shared_ptr< IThreadTask > Get_Thread_Task( void ) const { return ThreadTask; }
		bool Should_Return_Interface( void ) const { return ReturnInterface; }
		bool Should_Forward_Creator_Interface( void ) const { return ForwardCreatorInterface; }

	private:

		shared_ptr< IThreadTask > ThreadTask;

		bool ReturnInterface;
		bool ForwardCreatorInterface;
};

// Thread -> Manager
// Requests that a thread be rescheduled for execution
class CRescheduleThreadMessage : public IThreadMessage
{
	public:

		typedef IThreadMessage BASECLASS;
		
		CRescheduleThreadMessage( const SThreadKey &key, double reschedule_time ) :
			Key( key ),
			RescheduleTime( reschedule_time )
		{}

		virtual ~CRescheduleThreadMessage() {}

		const SThreadKey &Get_Key( void ) const { return Key; }
		double Get_Reschedule_Time( void ) const { return RescheduleTime; }

	private:

		SThreadKey Key;

		double RescheduleTime;
};

// Thread -> manager
// Requests that the manager shut down another thread
class CShutdownThreadMessage : public IThreadMessage
{
	public:

		typedef IThreadMessage BASECLASS;
		
		CShutdownThreadMessage( const SThreadKey &key ) :
			Key( key )
		{}

		virtual ~CShutdownThreadMessage() {}

		const SThreadKey &Get_Key( void ) const { return Key; }

	private:

		SThreadKey Key;
};

// Manager -> thread
// Tells a thread that another thread is going away soon, so stop using that thread's interface
class CShutdownInterfaceMessage : public IThreadMessage
{
	public:
		
		typedef IThreadMessage BASECLASS;

		CShutdownInterfaceMessage( const SThreadKey &key ) :
			Key( key )
		{}

		virtual ~CShutdownInterfaceMessage() {}

		const SThreadKey &Get_Key( void ) const { return Key; }

	private:

		SThreadKey Key;
};

// Thread -> Manager
// Tells the manager that a thread has stopped using another thread's interface
class CShutdownInterfaceAcknowledgement : public IThreadMessage
{
	public:
		
		typedef IThreadMessage BASECLASS;

		CShutdownInterfaceAcknowledgement( const SThreadKey &shutdown_key ) :
			ShutdownKey( shutdown_key )
		{}

		virtual ~CShutdownInterfaceAcknowledgement() {}

		const SThreadKey &Get_Shutdown_Key( void ) const { return ShutdownKey; }

	private:

		SThreadKey ShutdownKey;
};

// Manager -> thread
// Tells a thread that it should shut down and clean up any internal state
class CShutdownThreadRequest : public IThreadMessage
{
	public:
		
		typedef IThreadMessage BASECLASS;

		CShutdownThreadRequest( bool is_hard_shutdown ) :
			IsHardShutdown( is_hard_shutdown )
		{}

		virtual ~CShutdownThreadRequest() {}

		bool Get_Is_Hard_Shutdown( void ) const { return IsHardShutdown; }

	private:

		bool IsHardShutdown;
};

// thread -> manager
// Tells the manager that the sending thread has successfully shut down
class CShutdownThreadAcknowledgement : public IThreadMessage
{
	public:
		
		typedef IThreadMessage BASECLASS;

		CShutdownThreadAcknowledgement( void ) {}

		virtual ~CShutdownThreadAcknowledgement() {}

	private:

};

// Thread -> Manager
// Requests the entire concurrency system to be shut down
class CShutdownManagerMessage : public IThreadMessage
{
	public:

		typedef IThreadMessage BASECLASS;
		
		CShutdownManagerMessage( void ) {}

		virtual ~CShutdownManagerMessage() {}

	private:

};

#endif // THREAD_MANAGEMENT_MESSAGES_H