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

#include "ThreadProcessBase.h"

#include "IPPlatform/PlatformProcess.h"
#include "IPPlatform/PlatformThread.h"
#include "IPPlatform/PlatformTime.h"
#include "ProcessExecutionContext.h"
#include "ProcessExecutionMode.h"
#include "ProcessStatics.h"


CThreadProcessBase::CThreadProcessBase( const SProcessProperties &properties ) :
	BASECLASS( properties ),
	HasBeenRun( false )
{
}


CThreadProcessBase::~CThreadProcessBase()
{
}


EProcessExecutionMode::Enum CThreadProcessBase::Get_Execution_Mode( void ) const
{
	return EProcessExecutionMode::THREAD;
}


void CThreadProcessBase::Run( const CProcessExecutionContext &context )
{
	FATAL_ASSERT( !HasBeenRun );

	HasBeenRun = true;

	CPlatformThread *thread = context.Get_Platform_Thread();
	FATAL_ASSERT( thread != nullptr );

	thread->Create_And_Run( 0, ThreadExecutionFunctionType( this, &CThreadProcessBase::Thread_Function ), thread );
}


void CThreadProcessBase::Thread_Function( void *thread_data )
{
	CPlatformThread *thread = static_cast< CPlatformThread * >( thread_data );

	while ( !Is_Shutting_Down() )
	{
		CProcessExecutionContext context( thread );

		CProcessStatics::Set_Current_Process( this );
		BASECLASS::Run( context );
		CProcessStatics::Set_Current_Process( nullptr );
		Flush_System_Messages();

		NPlatform::Sleep( Get_Sleep_Interval_In_Milliseconds() );
	}
}


double CThreadProcessBase::Get_Current_Process_Time( void ) const
{
	return CPlatformTime::Convert_High_Resolution_Time_To_Seconds( CPlatformTime::Get_High_Resolution_Time() );
}
