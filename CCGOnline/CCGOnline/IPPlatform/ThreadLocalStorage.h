/**********************************************************************************************************************

	ThreadLocalStorage.h
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
