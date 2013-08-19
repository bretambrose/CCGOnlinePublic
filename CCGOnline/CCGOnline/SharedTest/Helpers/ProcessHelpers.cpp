/**********************************************************************************************************************

	ProcessHelpers.cpp
		defines unit tests for process related functionality

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

#include "stdafx.h"

#include "ProcessHelpers.h"

#include "Logging/LogInterface.h"
#include "Concurrency/ProcessConstants.h"
#include "Concurrency/ProcessExecutionContext.h"
#include "Concurrency/ProcessMailbox.h"
#include "Concurrency/ProcessStatics.h"
#include "Concurrency/TaskProcessBase.h"
#include "Concurrency/ThreadProcessBase.h"

bool CProcessBaseTester::HasProcessServiceExecuted = false;

CProcessBaseTester::CProcessBaseTester( CProcessBase *process ) :
	ManagerProxy( new CProcessMailbox( EProcessID::CONCURRENCY_MANAGER, MANAGER_PROCESS_PROPERTIES ) ),
	SelfProxy( new CProcessMailbox( AI_PROCESS_ID, process->Get_Properties() ) )
{
	process->Set_Manager_Mailbox( ManagerProxy->Get_Writable_Mailbox() );
	process->Set_My_Mailbox( SelfProxy->Get_Readable_Mailbox() );
	process->Initialize( AI_PROCESS_ID );
}

CProcessBaseTester::~CProcessBaseTester()
{
}

void CProcessBaseTester::Service( double time_seconds )
{
	CProcessStatics::Set_Current_Process( Get_Process() );

	CProcessExecutionContext context( nullptr, time_seconds );
	Get_Process()->Run( context );

	CProcessStatics::Set_Current_Process( nullptr );

	Get_Process()->Flush_System_Messages();
}

void CProcessBaseTester::Log( const std::wstring &log_string )
{
	CProcessStatics::Set_Current_Process( Get_Process() );
	CLogInterface::Log( log_string );
	CProcessStatics::Set_Current_Process( nullptr );
}

CProcessBase::FrameTableType &CProcessBaseTester::Get_Frame_Table( void ) const 
{ 
	return Get_Process()->PendingOutboundFrames; 
}

const CProcessBase::MailboxTableType &CProcessBaseTester::Get_Mailbox_Table( void ) const 
{ 
	return Get_Process()->Mailboxes; 
}

shared_ptr< CWriteOnlyMailbox > CProcessBaseTester::Get_Logging_Mailbox( void ) const 
{ 
	return Get_Process()->LoggingMailbox; 
}

void CProcessBaseTester::Set_Logging_Mailbox( shared_ptr< CWriteOnlyMailbox > mailbox ) 
{ 
	Get_Process()->LoggingMailbox = mailbox; 
}

shared_ptr< CWriteOnlyMailbox > CProcessBaseTester::Get_Manager_Mailbox( void ) const 
{ 
	return Get_Process()->ManagerMailbox; 
}

const unique_ptr< CProcessMessageFrame > &CProcessBaseTester::Get_Log_Frame( void ) const 
{ 
	return Get_Process()->LogFrame; 
}

const unique_ptr< CProcessMessageFrame > &CProcessBaseTester::Get_Manager_Frame( void ) const 
{ 
	return Get_Process()->ManagerFrame; 
}

bool CProcessBaseTester::Has_Frame( EProcessID::Enum id ) const
{
	return Get_Process()->PendingOutboundFrames.find( id ) != Get_Process()->PendingOutboundFrames.end();
}

const unique_ptr< CProcessMessageFrame > &CProcessBaseTester::Get_Frame( EProcessID::Enum id ) const
{
	return Get_Process()->PendingOutboundFrames.find( id )->second;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

CTaskProcessBaseTester::CTaskProcessBaseTester( CTaskProcessBase *process ) :
	BASECLASS( process ),
	TaskProcess( process )
{
}

CTaskProcessBaseTester::~CTaskProcessBaseTester()
{
}

shared_ptr< CTaskProcessBase > CTaskProcessBaseTester::Get_Task_Process( void ) const 
{ 
	return TaskProcess; 
}

double CTaskProcessBaseTester::Get_Reschedule_Interval( void ) const 
{ 
	return TaskProcess->Get_Reschedule_Interval(); 
}

CProcessBase *CTaskProcessBaseTester::Get_Process( void ) const 
{ 
	return TaskProcess.get(); 
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

CThreadProcessBaseTester::CThreadProcessBaseTester( CThreadProcessBase *process ) :
	BASECLASS( process ),
	ThreadProcess( process )
{
}

CThreadProcessBaseTester::~CThreadProcessBaseTester()
{
}

shared_ptr< CThreadProcessBase > CThreadProcessBaseTester::Get_Thread_Process( void ) const 
{ 
	return ThreadProcess; 
}

CProcessBase *CThreadProcessBaseTester::Get_Process( void ) const 
{ 
	return ThreadProcess.get(); 
}

