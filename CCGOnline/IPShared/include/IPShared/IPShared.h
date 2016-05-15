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

#include <IPCore/Always.h>

#ifdef PLATFORM_WINDOWS
    #pragma warning(disable : 4251)
    #ifdef USE_IMPORT_EXPORT
        #ifdef IPSHARED_EXPORTS
            #define  IPSHARED_API __declspec(dllexport)
        #else
            #define  IPSHARED_API __declspec(dllimport)
        #endif // IPSHARED_EXPORTS 
    #else
        #define IPSHARED_API
    #endif // USE_IMPORT_EXPORT
#else // PLATFORM_WINDOWS 
    #define IPSHARED_API
#endif

