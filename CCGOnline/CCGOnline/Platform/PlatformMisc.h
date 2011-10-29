/**********************************************************************************************************************

	[Placeholder for eventual source license]

	PlatformMisc.h
		A component that wraps miscellaneous OS-specific logic that isn't well classifiable

	(c) Copyright 2011, Bret Ambrose.  All rights reserved.

**********************************************************************************************************************/

#ifndef PLATFORM_MISC_H
#define PLATFORM_MISC_H

namespace NPlatform
{
	uint32 Get_Semi_Unique_ID( void );

	std::wstring Format_OS_Error_Message( uint32 error_code );
}

#endif // PLATFORM_MISC_H