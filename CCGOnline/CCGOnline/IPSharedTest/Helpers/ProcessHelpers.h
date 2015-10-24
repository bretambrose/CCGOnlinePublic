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

#include "IPShared/Concurrency/ProcessBase.h"
#include "IPShared/Concurrency/ProcessID.h"

namespace IP
{
namespace Execution
{

class CProcessMailbox;
class CTaskProcessBase;
class CThreadProcessBase;

static const EProcessID AI_PROCESS_ID = static_cast< EProcessID >( EProcessID::FIRST_FREE_ID );

} // namespace Execution
} // namespace IP


class CProcessBaseTester
{
	public:

		CProcessBaseTester( IP::Execution::CProcessBase *process );
		virtual ~CProcessBaseTester();

		void Service( double time_seconds );

		void Log( const std::wstring &log_string );

		std::shared_ptr< IP::Execution::CProcessMailbox > Get_Manager_Proxy( void ) const { return ManagerProxy; }
		std::shared_ptr< IP::Execution::CProcessMailbox > Get_Self_Proxy( void ) const { return SelfProxy; }

		static void Set_Has_Process_Service_Executed( void ) { HasProcessServiceExecuted = true; }
		static bool Get_Has_Process_Service_Executed( void ) { return HasProcessServiceExecuted; }

		bool Has_Frame( IP::Execution::EProcessID id ) const;
		const std::unique_ptr< IP::Execution::CProcessMessageFrame > &Get_Frame( IP::Execution::EProcessID id ) const;

		IP::Execution::CProcessBase::FrameTableType &Get_Frame_Table( void ) const;
		const IP::Execution::CProcessBase::MailboxTableType &Get_Mailbox_Table( void ) const;

		std::shared_ptr< IP::Execution::CWriteOnlyMailbox > Get_Logging_Mailbox( void ) const;
		void Set_Logging_Mailbox( std::shared_ptr< IP::Execution::CWriteOnlyMailbox > mailbox );

		std::shared_ptr< IP::Execution::CWriteOnlyMailbox > Get_Manager_Mailbox( void ) const;

		const std::unique_ptr< IP::Execution::CProcessMessageFrame > &Get_Log_Frame( void ) const;
		const std::unique_ptr< IP::Execution::CProcessMessageFrame > &Get_Manager_Frame( void ) const;

		virtual IP::Execution::CProcessBase *Get_Process( void ) const = 0;

	private:

		static bool HasProcessServiceExecuted;

		std::shared_ptr< IP::Execution::CProcessMailbox > ManagerProxy;
		std::shared_ptr< IP::Execution::CProcessMailbox > SelfProxy;
};

class CTaskProcessBaseTester : public CProcessBaseTester
{
	public:

		using BASECLASS = CProcessBaseTester;

		CTaskProcessBaseTester( IP::Execution::CTaskProcessBase *process );
		virtual ~CTaskProcessBaseTester();

		std::shared_ptr< IP::Execution::CTaskProcessBase > Get_Task_Process( void ) const;

		double Get_Reschedule_Interval( void ) const;

		virtual IP::Execution::CProcessBase *Get_Process( void ) const;

	private:

		std::shared_ptr< IP::Execution::CTaskProcessBase > TaskProcess;

};

class CThreadProcessBaseTester : public CProcessBaseTester
{
	public:

		using BASECLASS = CProcessBaseTester;

		CThreadProcessBaseTester( std::shared_ptr< IP::Execution::CThreadProcessBase > process );
		virtual ~CThreadProcessBaseTester();

		std::shared_ptr< IP::Execution::CThreadProcessBase > Get_Thread_Process( void ) const;

		virtual IP::Execution::CProcessBase *Get_Process( void ) const;

		void Start( void );

	private:

		std::shared_ptr< IP::Execution::CThreadProcessBase > ThreadProcess;

		std::unique_ptr< std::thread > ProcessThread;

};

