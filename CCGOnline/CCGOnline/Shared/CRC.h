/**********************************************************************************************************************

	[Placeholder for eventual source license]

	CRC.h
		A component defining several utility functions for calculating the CRC of a block of memory.  Primarily
		used to create unique integer keys for strings.  Will eventually be removed in favor of an external
		library with better hashing implementations.

	(c) Copyright 2011, Bret Ambrose.  All rights reserved.

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


