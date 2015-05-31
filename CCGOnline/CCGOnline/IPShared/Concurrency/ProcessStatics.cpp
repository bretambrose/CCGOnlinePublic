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

#include "ProcessStatics.h"

#include "ConcurrencyManager.h"
#include "ProcessInterface.h"
#include "IPPlatform/ThreadLocalStorage.h"

// Static member data definitions
uint32_t CProcessStatics::ProcessHandle = THREAD_LOCAL_INVALID_HANDLE;
uint32_t CProcessStatics::ConcurrencyManagerHandle = THREAD_LOCAL_INVALID_HANDLE;
bool CProcessStatics::Initialized = false;


void CProcessStatics::Initialize( void )
{
	if ( Initialized )
	{
		return;
	}

	FATAL_ASSERT( ProcessHandle == THREAD_LOCAL_INVALID_HANDLE );
	FATAL_ASSERT( ConcurrencyManagerHandle == THREAD_LOCAL_INVALID_HANDLE );

	ProcessHandle = CThreadLocalStorage::Allocate_Thread_Local_Storage();
	FATAL_ASSERT( ProcessHandle != THREAD_LOCAL_INVALID_HANDLE );

	ConcurrencyManagerHandle = CThreadLocalStorage::Allocate_Thread_Local_Storage();
	FATAL_ASSERT( ConcurrencyManagerHandle != THREAD_LOCAL_INVALID_HANDLE );

	Initialized = true;
}


void CProcessStatics::Shutdown( void )
{
	if ( !Initialized )
	{
		return;
	}

	Initialized = false;

	CThreadLocalStorage::Deallocate_Thread_Local_Storage( ProcessHandle );
	ProcessHandle = THREAD_LOCAL_INVALID_HANDLE;

	CThreadLocalStorage::Deallocate_Thread_Local_Storage( ConcurrencyManagerHandle );
	ConcurrencyManagerHandle = THREAD_LOCAL_INVALID_HANDLE;
}


void CProcessStatics::Set_Current_Process( IProcess *process )
{
	FATAL_ASSERT( Initialized );

	CThreadLocalStorage::Set_TLS_Value( ProcessHandle, process );
}


void CProcessStatics::Set_Concurrency_Manager( CConcurrencyManager *manager )
{
	FATAL_ASSERT( Initialized );

	CThreadLocalStorage::Set_TLS_Value( ConcurrencyManagerHandle, manager );
}


IProcess *CProcessStatics::Get_Current_Process( void )
{
	if ( !Initialized )
	{
		return nullptr;
	}

	return CThreadLocalStorage::Get_TLS_Value< IProcess >( ProcessHandle );
}


CConcurrencyManager *CProcessStatics::Get_Concurrency_Manager( void )
{
	if ( !Initialized )
	{
		return nullptr;
	}

	return CThreadLocalStorage::Get_TLS_Value< CConcurrencyManager >( ConcurrencyManagerHandle );
}
