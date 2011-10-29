/**********************************************************************************************************************

	[Placeholder for eventual source license]

	PlatformMisc.h
		A component that wraps miscellaneous OS-specific process functionality

	(c) Copyright 2011, Bret Ambrose.  All rights reserved.

**********************************************************************************************************************/

#ifndef PLATFORM_PROCESS_H
#define PLATFORM_PROCESS_H

namespace NPlatform
{
	std::wstring Get_Exe_Name( void );
	std::wstring Get_Service_Name( void );

	uint32 Get_Self_Process_ID( void );

	void Sleep( uint32 milliseconds );
}

#endif // PLATFORM_PROCESS_H
