/**********************************************************************************************************************

	PlatformMisc.h
		A component that wraps miscellaneous OS-specific process functionality

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
