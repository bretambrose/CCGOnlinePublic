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
#include "IPPlatform/PlatformTime.h"
#include "ProcessExecutionContext.h"
#include "ProcessStatics.h"

namespace IP
{
namespace Execution
{

CThreadProcessBase::CThreadProcessBase( const SProcessProperties &properties ) :
	BASECLASS( properties ),
	StartLock(),
	ExecutionThread( nullptr )
{
}

CThreadProcessBase::~CThreadProcessBase()
{
}

void CThreadProcessBase::Run( const CProcessExecutionContext &context )
{
	IP_UNREFERENCED_PARAM( context );

	std::lock_guard< std::mutex > startLock( StartLock );

	if ( ExecutionThread == nullptr )
	{
		ExecutionThread = std::make_unique< std::thread >( std::bind( &CThreadProcessBase::Thread_Function, this ) );
	}
}

void CThreadProcessBase::Finalize( void )
{
	if ( ExecutionThread && ExecutionThread->joinable() )
	{
		ExecutionThread->join();
	}
}

void CThreadProcessBase::Thread_Function( void )
{
	while ( !Is_Shutting_Down() )
	{
		CProcessExecutionContext context;

		CProcessStatics::Set_Current_Process( this );
		BASECLASS::Run( context );
		CProcessStatics::Set_Current_Process( nullptr );
		Flush_System_Messages();

		std::this_thread::sleep_for( std::chrono::milliseconds( Get_Sleep_Interval_In_Milliseconds() ) );
	}
}


double CThreadProcessBase::Get_Current_Process_Time( void ) const
{
	return IP::Time::Convert_Duration_To_Seconds( IP::Time::Get_Elapsed_System_Time() );
}

} // namespace Execution
} // namespace IP