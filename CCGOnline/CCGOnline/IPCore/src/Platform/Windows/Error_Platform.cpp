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

#include <IPCore/Platform/Windows/Error.h>

#include <IPCore/System/WindowsWrapper.h>

namespace IP
{
namespace Error
{

IP::String Format_OS_Error_Message( uint32_t error_code )
{
	char msg_buffer[ 1024 ];

	::FormatMessage( FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
						  nullptr,
						  error_code,
						  MAKELANGID( LANG_NEUTRAL, SUBLANG_DEFAULT ),
						  (LPTSTR) &msg_buffer,
						  sizeof( msg_buffer ), 
						  nullptr );

	return IP::String( msg_buffer );
}

} // namespace Error
} // namespace IP

