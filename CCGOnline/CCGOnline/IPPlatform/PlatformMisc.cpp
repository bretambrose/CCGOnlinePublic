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

#include "stdafx.h"

#include "PlatformMisc.h"

static void Convert_GUID_To_Numeric( const GUID &guid, uint32_t &first32, uint32_t &second32, uint32_t &third32, uint32_t &fourth32 )
{
	first32 = guid.Data1;
	
	second32 = guid.Data2;
	second32 <<= 16;
	second32 |= guid.Data3;

	third32 = 0;
	for ( uint32_t i = 0; i < 4; i++ )
	{
		third32 |= guid.Data4[ i ];
		third32 <<= 8;
	}

	fourth32 = 0;
	for ( uint32_t i = 4; i < 8; i++ )
	{
		fourth32 |= guid.Data4[ i ];
		fourth32 <<= 8;
	}
}

namespace IP
{
namespace Misc
{

uint32_t Get_Semi_Unique_ID( void )
{
	GUID guid;
	::CoCreateGuid( &guid ); 

	uint32_t part1, part2, part3, part4;
	::Convert_GUID_To_Numeric( guid, part1, part2, part3, part4 );

	return part1 ^ part2 ^ part3 ^ part4;
}

std::wstring Format_OS_Error_Message( uint32_t error_code )
{
	wchar_t msg_buffer[ 1024 ];

	::FormatMessageW( FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
							nullptr,
							error_code,
							MAKELANGID( LANG_NEUTRAL, SUBLANG_DEFAULT ),
							(LPTSTR) &msg_buffer,
							sizeof( msg_buffer ), 
							nullptr );

	return std::wstring( msg_buffer );
}

} // namespace Misc
} // namespace IP