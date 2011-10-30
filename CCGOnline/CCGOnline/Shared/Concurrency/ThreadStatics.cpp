/**********************************************************************************************************************

	ThreadStatics.h
		A component containing a static class that manages the thread-local variables that hold handles to
		the executing task and the concurrency manager.

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

#include "ThreadStatics.h"

#include "ConcurrencyManager.h"
#include "ThreadTaskInterface.h"
#include "ThreadLocalStorage.h"

// Static member data definitions
uint32 CThreadStatics::ThreadHandle = THREAD_LOCAL_INVALID_HANDLE;
uint32 CThreadStatics::ConcurrencyManagerHandle = THREAD_LOCAL_INVALID_HANDLE;
bool CThreadStatics::Initialized = false;

/**********************************************************************************************************************
	CThreadStatics::Initialize -- Initialize the thread local storage needed
					
**********************************************************************************************************************/
void CThreadStatics::Initialize( void )
{
	if ( Initialized )
	{
		return;
	}

	FATAL_ASSERT( ThreadHandle == THREAD_LOCAL_INVALID_HANDLE );
	FATAL_ASSERT( ConcurrencyManagerHandle == THREAD_LOCAL_INVALID_HANDLE );

	ThreadHandle = CThreadLocalStorage::Allocate_Thread_Local_Storage();
	FATAL_ASSERT( ThreadHandle != THREAD_LOCAL_INVALID_HANDLE );

	ConcurrencyManagerHandle = CThreadLocalStorage::Allocate_Thread_Local_Storage();
	FATAL_ASSERT( ConcurrencyManagerHandle != THREAD_LOCAL_INVALID_HANDLE );

	Initialized = true;
}

/**********************************************************************************************************************
	CThreadStatics::Initialize -- Cleans up the thread local storage used
					
**********************************************************************************************************************/
void CThreadStatics::Shutdown( void )
{
	if ( !Initialized )
	{
		return;
	}

	Initialized = false;

	CThreadLocalStorage::Deallocate_Thread_Local_Storage( ThreadHandle );
	ThreadHandle = THREAD_LOCAL_INVALID_HANDLE;

	CThreadLocalStorage::Deallocate_Thread_Local_Storage( ConcurrencyManagerHandle );
	ConcurrencyManagerHandle = THREAD_LOCAL_INVALID_HANDLE;
}

/**********************************************************************************************************************
	CThreadStatics::Set_Current_Thread_Task -- sets the current executing thread task variable

		thread_task -- the currently executing thread task
					
**********************************************************************************************************************/
void CThreadStatics::Set_Current_Thread_Task( IThreadTask *thread_task )
{
	FATAL_ASSERT( Initialized );

	CThreadLocalStorage::Set_TLS_Value( ThreadHandle, thread_task );
}

/**********************************************************************************************************************
	CThreadStatics::Set_Concurrency_Manager -- sets the concurrency manager variable

		manager -- the global concurrency manager
					
**********************************************************************************************************************/
void CThreadStatics::Set_Concurrency_Manager( CConcurrencyManager *manager )
{
	FATAL_ASSERT( Initialized );

	CThreadLocalStorage::Set_TLS_Value( ConcurrencyManagerHandle, manager );
}

/**********************************************************************************************************************
	CThreadStatics::Get_Current_Thread_Task -- gets the current executing thread task variable

		Returns: the currently executing thread task
					
**********************************************************************************************************************/
IThreadTask *CThreadStatics::Get_Current_Thread_Task( void )
{
	if ( !Initialized )
	{
		return nullptr;
	}

	return CThreadLocalStorage::Get_TLS_Value< IThreadTask >( ThreadHandle );
}

/**********************************************************************************************************************
	CThreadStatics::Get_Concurrency_Manager -- gets the concurrency manager variable

		Returns: the global concurrency manager
					
**********************************************************************************************************************/
CConcurrencyManager *CThreadStatics::Get_Concurrency_Manager( void )
{
	if ( !Initialized )
	{
		return nullptr;
	}

	return CThreadLocalStorage::Get_TLS_Value< CConcurrencyManager >( ConcurrencyManagerHandle );
}
