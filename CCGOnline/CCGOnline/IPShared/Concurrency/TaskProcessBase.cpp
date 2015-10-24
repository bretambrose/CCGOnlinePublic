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

#include "stdafx.h"

#include "TaskProcessBase.h"

#include "ProcessExecutionContext.h"
#include "Messaging/ProcessManagementMessages.h"

namespace IP
{
namespace Execution
{

CTaskProcessBase::CTaskProcessBase( const SProcessProperties &properties ) :
	BASECLASS( properties ),
	FirstServiceTimeSeconds( 0.0 ),
	CurrentTimeSeconds( 0.0 ),
	HasBeenRun( false )
{
}


CTaskProcessBase::~CTaskProcessBase()
{
}

double CTaskProcessBase::Get_Current_Process_Time( void ) const
{
	return CurrentTimeSeconds;
}


double CTaskProcessBase::Get_Reschedule_Interval( void ) const
{
	return .1;
}


double CTaskProcessBase::Get_Reschedule_Time( void ) const
{
	return std::min( Get_Current_Process_Time() + Get_Reschedule_Interval(), Get_Next_Task_Time() );
}


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


void CTaskProcessBase::Service_Reschedule( void )
{
	if ( Should_Reschedule() )
	{
		std::unique_ptr< const Messaging::IProcessMessage > reschedule_msg( std::make_unique< Messaging::CRescheduleProcessMessage >( Get_Reschedule_Time() ) );
		Send_Manager_Message( reschedule_msg );
	}
}

} // Execution
} // IP