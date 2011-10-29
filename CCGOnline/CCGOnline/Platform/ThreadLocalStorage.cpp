/**********************************************************************************************************************

	[Placeholder for eventual source license]

	ThreadLocalStorage.cpp
		A component that allocated and manages thread local storage

	(c) Copyright 2011, Bret Ambrose.  All rights reserved.

**********************************************************************************************************************/

#include "stdafx.h"

#include "ThreadLocalStorage.h"

#ifdef WIN32

/**********************************************************************************************************************
	CThreadLocalStorage::Allocate_Thread_Local_Storage -- allocates a pointer-wide element of thread local storage

		Returns: integer handle to the allocated storage

**********************************************************************************************************************/
uint32 CThreadLocalStorage::Allocate_Thread_Local_Storage( void )
{
	uint32 tls_handle = TlsAlloc();
	TlsSetValue( tls_handle, nullptr );

	return tls_handle;
}

/**********************************************************************************************************************
	CThreadLocalStorage::Deallocate_Thread_Local_Storage -- deaallocates a previously allocated element of thread local 
		storage

**********************************************************************************************************************/
void CThreadLocalStorage::Deallocate_Thread_Local_Storage( uint32 tls_handle )
{
	TlsFree( tls_handle );
}		

/**********************************************************************************************************************
	CThreadLocalStorage::Get_Raw_TLS_Value -- gets the value held in a slot of thread local storage

		tls_handle -- allocated id of the slot to query

		Returns: pointer-wide value held in the tls slot

**********************************************************************************************************************/
void *CThreadLocalStorage::Get_Raw_TLS_Value( uint32 tls_handle )
{
	return TlsGetValue( tls_handle );
}

/**********************************************************************************************************************
	CThreadLocalStorage::Set_Raw_TLS_Value -- sets the value held in a slot of thread local storage

		tls_handle -- allocated id of the slot to modify
		handle -- new value for the slot to hold

**********************************************************************************************************************/
void CThreadLocalStorage::Set_Raw_TLS_Value( uint32 tls_handle, void *handle )
{
	TlsSetValue( tls_handle, handle );
}

#endif // WIN32