/**********************************************************************************************************************

	[Placeholder for eventual source license]

	ThreadLocalStorage.h
		A component that allocated and manages thread local storage

	(c) Copyright 2011, Bret Ambrose.  All rights reserved.

**********************************************************************************************************************/

#ifndef THREAD_LOCAL_STORAGE_H
#define THREAD_LOCAL_STORAGE_H

#ifdef WIN32

#define THREAD_LOCAL_INVALID_HANDLE ( ( uint32 ) 0xFFFFFFFF )

#else

#define THREAD_LOCAL_INVALID_HANDLE ??

#endif

// Static class with an interface for managing thread local storage.  Writing to TLS is not thread safe by design.
class CThreadLocalStorage
{
	public:

		static uint32 Allocate_Thread_Local_Storage( void );
		static void Deallocate_Thread_Local_Storage( uint32 tls_handle );
		
		template< class T >
		static void Set_TLS_Value( uint32 tls_handle, T *value )
		{
			Set_Raw_TLS_Value( tls_handle, static_cast< void * >( value ) );
		}

		template< class T >
		static T *Get_TLS_Value( uint32 tls_handle )
		{
			return static_cast< T * >( Get_Raw_TLS_Value( tls_handle ) );
		}

	private:

		static void *Get_Raw_TLS_Value( uint32 tls_handle );
		static void Set_Raw_TLS_Value( uint32 tls_handle, void *handle );

};

#endif // THREAD_LOCAL_STORAGE_H
