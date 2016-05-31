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

#pragma once

namespace IP
{
namespace TLS
{

	bool Is_Valid_Handle( uint32_t tls_handle );

	uint32_t Allocate_Thread_Local_Storage( void );
	void Deallocate_Thread_Local_Storage( uint32_t tls_handle );
		
	template< class T >
	void Set_TLS_Value( uint32_t tls_handle, T *value )
	{
		Set_Raw_TLS_Value( tls_handle, static_cast< void * >( value ) );
	}

	template< class T >
	T *Get_TLS_Value( uint32_t tls_handle )
	{
		return static_cast< T * >( Get_Raw_TLS_Value( tls_handle ) );
	}

	void *Get_Raw_TLS_Value( uint32_t tls_handle );
	void Set_Raw_TLS_Value( uint32_t tls_handle, void *handle );


} // namespace TLS
} // namespace IP

#if PLATFORM_WINDOWS

#define G_THREAD_LOCAL __declspec( thread )

#define THREAD_LOCAL_INVALID_HANDLE ((uint32_t)-1)

#else

#define G_THREAD_LOCAL thread_local

#endif

