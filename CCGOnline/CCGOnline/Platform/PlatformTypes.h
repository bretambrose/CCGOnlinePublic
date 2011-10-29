/**********************************************************************************************************************

	[Placeholder for eventual source license]

	PlatformTypes.h
		Type definitions for basic numeric types.  These typedefs should always be used over the builtins.

	(c) Copyright 2011, Bret Ambrose.  All rights reserved.

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
