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

#ifndef PROCESS_HELPERS_H
#define PROCESS_HELPERS_H

#include "IPShared/Concurrency/ProcessBase.h"
#include "IPShared/Concurrency/ProcessID.h"

class CProcessMailbox;
class CTaskProcessBase;
class CThreadProcessBase;

static const EProcessID::Enum AI_PROCESS_ID = static_cast< EProcessID::Enum >( EProcessID::FIRST_FREE_ID );

class CProcessBaseTester
{
	public:

		CProcessBaseTester( CProcessBase *process );
		virtual ~CProcessBaseTester();

		void Service( double time_seconds );

		void Log( const std::wstring &log_string );

		std::shared_ptr< CProcessMailbox > Get_Manager_Proxy( void ) const { return ManagerProxy; }
		std::shared_ptr< CProcessMailbox > Get_Self_Proxy( void ) const { return SelfProxy; }

		static void Set_Has_Process_Service_Executed( void ) { HasProcessServiceExecuted = true; }
		static bool Get_Has_Process_Service_Executed( void ) { return HasProcessServiceExecuted; }

		bool Has_Frame( EProcessID::Enum id ) const;
		const std::unique_ptr< CProcessMessageFrame > &Get_Frame( EProcessID::Enum id ) const;

		CProcessBase::FrameTableType &Get_Frame_Table( void ) const;
		const CProcessBase::MailboxTableType &Get_Mailbox_Table( void ) const;

		std::shared_ptr< CWriteOnlyMailbox > Get_Logging_Mailbox( void ) const;
		void Set_Logging_Mailbox( std::shared_ptr< CWriteOnlyMailbox > mailbox );

		std::shared_ptr< CWriteOnlyMailbox > Get_Manager_Mailbox( void ) const;

		const std::unique_ptr< CProcessMessageFrame > &Get_Log_Frame( void ) const;
		const std::unique_ptr< CProcessMessageFrame > &Get_Manager_Frame( void ) const;

		virtual CProcessBase *Get_Process( void ) const = 0;

	private:

		static bool HasProcessServiceExecuted;

		std::shared_ptr< CProcessMailbox > ManagerProxy;
		std::shared_ptr< CProcessMailbox > SelfProxy;
};

class CTaskProcessBaseTester : public CProcessBaseTester
{
	public:

		typedef CProcessBaseTester BASECLASS;

		CTaskProcessBaseTester( CTaskProcessBase *process );
		virtual ~CTaskProcessBaseTester();

		std::shared_ptr< CTaskProcessBase > Get_Task_Process( void ) const;

		double Get_Reschedule_Interval( void ) const;

		virtual CProcessBase *Get_Process( void ) const;

	private:

		std::shared_ptr< CTaskProcessBase > TaskProcess;

};

class CThreadProcessBaseTester : public CProcessBaseTester
{
	public:

		typedef CProcessBaseTester BASECLASS;

		CThreadProcessBaseTester( CThreadProcessBase *process );
		virtual ~CThreadProcessBaseTester();

		std::shared_ptr< CThreadProcessBase > Get_Thread_Process( void ) const;

		virtual CProcessBase *Get_Process( void ) const;

	private:

		std::shared_ptr< CThreadProcessBase > ThreadProcess;

};

#endif // PROCESS_HELPERS_H
