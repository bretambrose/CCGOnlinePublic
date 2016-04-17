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

#include <IPCore/Process/ThreadLocalStorage.h>

#include <IPCore/System/WindowsWrapper.h>

namespace IP
{
namespace TLS
{

// #define THREAD_LOCAL_INVALID_HANDLE ( ( uint32_t ) 0xFFFFFFFF )

bool Is_Valid_Handle( uint32_t tls_handle )
{
	return tls_handle != THREAD_LOCAL_INVALID_HANDLE;
}

uint32_t Allocate_Thread_Local_Storage( void )
{
	uint32_t tls_handle = TlsAlloc();
	TlsSetValue( tls_handle, nullptr );

	return tls_handle;
}


void Deallocate_Thread_Local_Storage( uint32_t tls_handle )
{
	TlsFree( tls_handle );
}		


void *Get_Raw_TLS_Value( uint32_t tls_handle )
{
	return TlsGetValue( tls_handle );
}


void Set_Raw_TLS_Value( uint32_t tls_handle, void *handle )
{
	TlsSetValue( tls_handle, handle );
}

} // namespace TLS
} // namespace IP
