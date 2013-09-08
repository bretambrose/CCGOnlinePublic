/**********************************************************************************************************************

	ProcessStatics.h
		A component containing a static class that manages the thread-local variables that hold handles to
		the executing process and the concurrency manager.

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
uint32 CProcessStatics::ProcessHandle = THREAD_LOCAL_INVALID_HANDLE;
uint32 CProcessStatics::ConcurrencyManagerHandle = THREAD_LOCAL_INVALID_HANDLE;
bool CProcessStatics::Initialized = false;

/**********************************************************************************************************************
	CProcessStatics::Initialize -- Initialize the thread local storage needed
					
**********************************************************************************************************************/
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

/**********************************************************************************************************************
	CProcessStatics::Initialize -- Cleans up the thread local storage used
					
**********************************************************************************************************************/
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

/**********************************************************************************************************************
	CProcessStatics::Set_Current_Thread_Task -- sets the current executing thread task variable

		thread_task -- the currently executing thread task
					
**********************************************************************************************************************/
void CProcessStatics::Set_Current_Process( IProcess *process )
{
	FATAL_ASSERT( Initialized );

	CThreadLocalStorage::Set_TLS_Value( ProcessHandle, process );
}

/**********************************************************************************************************************
	CProcessStatics::Set_Concurrency_Manager -- sets the concurrency manager variable

		manager -- the global concurrency manager
					
**********************************************************************************************************************/
void CProcessStatics::Set_Concurrency_Manager( CConcurrencyManager *manager )
{
	FATAL_ASSERT( Initialized );

	CThreadLocalStorage::Set_TLS_Value( ConcurrencyManagerHandle, manager );
}

/**********************************************************************************************************************
	CProcessStatics::Get_Current_Process -- gets the current executing process

		Returns: the currently executing process or null
					
**********************************************************************************************************************/
IProcess *CProcessStatics::Get_Current_Process( void )
{
	if ( !Initialized )
	{
		return nullptr;
	}

	return CThreadLocalStorage::Get_TLS_Value< IProcess >( ProcessHandle );
}

/**********************************************************************************************************************
	CProcessStatics::Get_Concurrency_Manager -- gets the concurrency manager variable

		Returns: the global concurrency manager
					
**********************************************************************************************************************/
CConcurrencyManager *CProcessStatics::Get_Concurrency_Manager( void )
{
	if ( !Initialized )
	{
		return nullptr;
	}

	return CThreadLocalStorage::Get_TLS_Value< CConcurrencyManager >( ConcurrencyManagerHandle );
}
