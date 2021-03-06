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

// A class that wraps a variety of time-related functionality that is os-specific. 
namespace IP
{
namespace Time
{
	// Types
	using SystemTimePoint = std::chrono::system_clock::time_point;
	using SystemDuration = std::chrono::system_clock::duration;

	// Interface
	SystemTimePoint Get_Current_System_Time( void );
	SystemDuration Get_Elapsed_System_Time( void );
	SystemTimePoint Get_File_Last_Modified_Time( const std::wstring &file_name );

	double Convert_Duration_To_Seconds( SystemDuration duration );

	std::wstring Format_System_Time( SystemTimePoint time_point );

} // namespace Time
} // namespace IP

