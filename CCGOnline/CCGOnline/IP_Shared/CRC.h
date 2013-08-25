/**********************************************************************************************************************

	CRC.h
		A component defining several utility functions for calculating the CRC of a block of memory.  Primarily
		used to create unique integer keys for strings.  Will eventually be removed in favor of an external
		library with better hashing implementations.

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

#ifndef CRC_H
#define CRC_H

#include "CRCValue.h"

namespace NCRCUtils
{	
	CRCValue CRC_Memory( const void *memory, size_t length );

	CRCValue String_To_CRC( const std::string &value );
	CRCValue String_To_CRC_Case_Insensitive( const std::string &value );
				
	CRCValue String_To_CRC( const std::wstring &value );
	CRCValue String_To_CRC_Case_Insensitive( const std::wstring &value );

};

#endif // CRC_H


