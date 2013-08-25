/**********************************************************************************************************************

	PlatformTypes.h
		Type definitions for basic numeric types.  These typedefs should always be used over the builtins.

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

#ifndef PLATFORM_TYPES_H
#define PLATFORM_TYPES_H

#ifdef _DEBUG

#define TBB_USE_DEBUG 1

#endif

#ifdef WIN32

typedef unsigned __int64 uint64;
typedef __int64 int64;
typedef unsigned int uint32;
typedef int int32;
typedef unsigned short uint16;
typedef short int16;
typedef unsigned char uint8;
typedef char int8;
 
#else

typedef unsigned long long uint64;
typedef long long int64;
typedef unsigned int uint32;
typedef int int32;
typedef unsigned short uint16;
typedef short int16;
typedef unsigned char uint8;
typedef char int8;

#endif

typedef uint64 StringCRC64;
typedef uint32 StringCRC32;

#endif // PLATFORM_TYPES_H
