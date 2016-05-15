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

#include <IPShared/IPShared.h>

#include <IPCore/Memory/Stl/String.h>
#include <IPShared/CRCValue.h>

namespace IP
{	
namespace CRC
{

	IPSHARED_API CRCValue CRC_Memory( const void *memory, size_t length );

	IPSHARED_API CRCValue String_To_CRC( const IP::String &value );
	IPSHARED_API CRCValue String_To_CRC_Case_Insensitive( const IP::String &value );

} // namespace CRC
} // namespace IP



