/**********************************************************************************************************************

	VirtualProcessStatics.h
		A component containing a static class that manages the thread-local variables that hold handles to
		the executing virtual process and the concurrency manager.

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

#include "VirtualProcessStatics.h"

#include "ConcurrencyManager.h"
#include "VirtualProcessInterface.h"
#include "ThreadLocalStorage.h"

// Static member data definitions
uint32 CVirtualProcessStatics::VirtualProcessHandle = THREAD_LOCAL_INVALID_HANDLE;
uint32 CVirtualProcessStatics::ConcurrencyManagerHandle = THREAD_LOCAL_INVALID_HANDLE;
bool CVirtualProcessStatics::Initialized = false;

/**********************************************************************************************************************
	CVirtualProcessStatics::Initialize -- Initialize the thread local storage needed
					
**********************************************************************************************************************/
void CVirtualProcessStatics::Initialize( void )
{
	if ( Initialized )
	{
		return;
	}

	FATAL_ASSERT( VirtualProcessHandle == THREAD_LOCAL_INVALID_HANDLE );
	FATAL_ASSERT( ConcurrencyManagerHandle == THREAD_LOCAL_INVALID_HANDLE );

	VirtualProcessHandle = CThreadLocalStorage::Allocate_Thread_Local_Storage();
	FATAL_ASSERT( VirtualProcessHandle != THREAD_LOCAL_INVALID_HANDLE );

	ConcurrencyManagerHandle = CThreadLocalStorage::Allocate_Thread_Local_Storage();
	FATAL_ASSERT( ConcurrencyManagerHandle != THREAD_LOCAL_INVALID_HANDLE );

	Initialized = true;
}

/**********************************************************************************************************************
	CVirtualProcessStatics::Initialize -- Cleans up the thread local storage used
					
**********************************************************************************************************************/
void CVirtualProcessStatics::Shutdown( void )
{
	if ( !Initialized )
	{
		return;
	}

	Initialized = false;

	CThreadLocalStorage::Deallocate_Thread_Local_Storage( VirtualProcessHandle );
	VirtualProcessHandle = THREAD_LOCAL_INVALID_HANDLE;

	CThreadLocalStorage::Deallocate_Thread_Local_Storage( ConcurrencyManagerHandle );
	ConcurrencyManagerHandle = THREAD_LOCAL_INVALID_HANDLE;
}

/**********************************************************************************************************************
	CVirtualProcessStatics::Set_Current_Thread_Task -- sets the current executing thread task variable

		thread_task -- the currently executing thread task
					
**********************************************************************************************************************/
void CVirtualProcessStatics::Set_Current_Virtual_Process( IVirtualProcess *virtual_process )
{
	FATAL_ASSERT( Initialized );

	CThreadLocalStorage::Set_TLS_Value( VirtualProcessHandle, virtual_process );
}

/**********************************************************************************************************************
	CVirtualProcessStatics::Set_Concurrency_Manager -- sets the concurrency manager variable

		manager -- the global concurrency manager
					
**********************************************************************************************************************/
void CVirtualProcessStatics::Set_Concurrency_Manager( CConcurrencyManager *manager )
{
	FATAL_ASSERT( Initialized );

	CThreadLocalStorage::Set_TLS_Value( ConcurrencyManagerHandle, manager );
}

/**********************************************************************************************************************
	CVirtualProcessStatics::Get_Current_Thread_Task -- gets the current executing thread task variable

		Returns: the currently executing thread task
					
**********************************************************************************************************************/
IVirtualProcess *CVirtualProcessStatics::Get_Current_Virtual_Process( void )
{
	if ( !Initialized )
	{
		return nullptr;
	}

	return CThreadLocalStorage::Get_TLS_Value< IVirtualProcess >( VirtualProcessHandle );
}

/**********************************************************************************************************************
	CVirtualProcessStatics::Get_Concurrency_Manager -- gets the concurrency manager variable

		Returns: the global concurrency manager
					
**********************************************************************************************************************/
CConcurrencyManager *CVirtualProcessStatics::Get_Concurrency_Manager( void )
{
	if ( !Initialized )
	{
		return nullptr;
	}

	return CThreadLocalStorage::Get_TLS_Value< CConcurrencyManager >( ConcurrencyManagerHandle );
}
