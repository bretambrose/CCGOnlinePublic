/**********************************************************************************************************************

	[Placeholder for eventual source license]

	WindowsWrapper.h
		A wrapper for the windows header files that turns off the conflicting min/max functions

	(c) Copyright 2011, Bret Ambrose.  All rights reserved.

**********************************************************************************************************************/

#ifndef WINDOWS_WRAPPER_H
#define WINDOWS_WRAPPER_H

#define NOMINMAX
#include <Windows.h>

#endif // WINDOWS_WRAPPER_H
