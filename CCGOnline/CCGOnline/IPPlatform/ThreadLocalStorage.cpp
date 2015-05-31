/**********************************************************************************************************************

	ThreadLocalStorage.cpp
		A component that allocated and manages thread local storage

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

#include "ThreadLocalStorage.h"

#ifdef WIN32

/**********************************************************************************************************************
	CThreadLocalStorage::Allocate_Thread_Local_Storage -- allocates a pointer-wide element of thread local storage

		Returns: integer handle to the allocated storage

**********************************************************************************************************************/
uint32_t CThreadLocalStorage::Allocate_Thread_Local_Storage( void )
{
	uint32_t tls_handle = TlsAlloc();
	TlsSetValue( tls_handle, nullptr );

	return tls_handle;
}

/**********************************************************************************************************************
	CThreadLocalStorage::Deallocate_Thread_Local_Storage -- deaallocates a previously allocated element of thread local 
		storage

**********************************************************************************************************************/
void CThreadLocalStorage::Deallocate_Thread_Local_Storage( uint32_t tls_handle )
{
	TlsFree( tls_handle );
}		

/**********************************************************************************************************************
	CThreadLocalStorage::Get_Raw_TLS_Value -- gets the value held in a slot of thread local storage

		tls_handle -- allocated id of the slot to query

		Returns: pointer-wide value held in the tls slot

**********************************************************************************************************************/
void *CThreadLocalStorage::Get_Raw_TLS_Value( uint32_t tls_handle )
{
	return TlsGetValue( tls_handle );
}

/**********************************************************************************************************************
	CThreadLocalStorage::Set_Raw_TLS_Value -- sets the value held in a slot of thread local storage

		tls_handle -- allocated id of the slot to modify
		handle -- new value for the slot to hold

**********************************************************************************************************************/
void CThreadLocalStorage::Set_Raw_TLS_Value( uint32_t tls_handle, void *handle )
{
	TlsSetValue( tls_handle, handle );
}

#endif // WIN32