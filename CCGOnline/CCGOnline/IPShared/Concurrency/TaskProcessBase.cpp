/**********************************************************************************************************************

	TaskProcessBase.cpp
		A component containing the logic shared by all task-based processes.

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

#include "TaskProcessBase.h"

#include "ProcessExecutionMode.h"
#include "ProcessExecutionContext.h"
#include "Messaging/ProcessManagementMessages.h"

/**********************************************************************************************************************
	CTaskProcessBase::CTaskProcessBase -- constructor
	
		properties -- the properties of this process
				
**********************************************************************************************************************/
CTaskProcessBase::CTaskProcessBase( const SProcessProperties &properties ) :
	BASECLASS( properties ),
	FirstServiceTimeSeconds( 0.0 ),
	CurrentTimeSeconds( 0.0 ),
	HasBeenRun( false )
{
}

/**********************************************************************************************************************
	CTaskProcessBase::~CTaskProcessBase -- destructor
					
**********************************************************************************************************************/
CTaskProcessBase::~CTaskProcessBase()
{
}

/**********************************************************************************************************************
	CTaskProcessBase::Get_Execution_Mode -- gets the execution mode (task-based or thread-based) of the process

		Returns: execution mode of the process
					
**********************************************************************************************************************/
EProcessExecutionMode::Enum CTaskProcessBase::Get_Execution_Mode( void ) const
{
	return EProcessExecutionMode::TASK;
}

/**********************************************************************************************************************
	CTaskProcessBase::Get_Current_Process_Time -- gets the current execution time in seconds

		Returns: current execution time
					
**********************************************************************************************************************/
double CTaskProcessBase::Get_Current_Process_Time( void ) const
{
	return CurrentTimeSeconds;
}

/**********************************************************************************************************************
	CTaskProcessBase::Get_Reschedule_Interval -- gets the reschedule interval in seconds

		Returns: reschedule interval
					
**********************************************************************************************************************/
double CTaskProcessBase::Get_Reschedule_Interval( void ) const
{
	return .1;
}

/**********************************************************************************************************************
	CTaskProcessBase::Get_Reschedule_Time -- gets the execution time that this process should be run again at, in seconds

		Returns: next execution time
					
**********************************************************************************************************************/
double CTaskProcessBase::Get_Reschedule_Time( void ) const
{
	return std::min( Get_Current_Process_Time() + Get_Reschedule_Interval(), Get_Next_Task_Time() );
}

/**********************************************************************************************************************
	CTaskProcessBase::Run -- one-time execution logic

		context -- the execution context that this process is being run under
					
**********************************************************************************************************************/
void CTaskProcessBase::Run( const CProcessExecutionContext &context )
{
	if ( !HasBeenRun )
	{
		HasBeenRun = true;
		FirstServiceTimeSeconds = context.Get_Elapsed_Time();
	}

	CurrentTimeSeconds = context.Get_Elapsed_Time();

	BASECLASS::Run( context );
}

/**********************************************************************************************************************
	CTaskProcessBase::Service_Reschedule -- reschedules the process, if appropriate
					
**********************************************************************************************************************/
void CTaskProcessBase::Service_Reschedule( void )
{
	if ( Should_Reschedule() )
	{
		unique_ptr< const IProcessMessage > reschedule_msg( new CRescheduleProcessMessage( Get_Reschedule_Time() ) );
		Send_Manager_Message( reschedule_msg );
	}
}