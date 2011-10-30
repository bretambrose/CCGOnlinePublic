/**********************************************************************************************************************

	PlatformTime.h
		A component that wraps miscellaneous OS-specific time functionality

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

#ifndef PLATFORM_TIME_H
#define PLATFORM_TIME_H

// A class that wraps a variety of time-related functionality that is os-specific.  
class CPlatformTime
{
	public:

		static void Initialize( void );

		// High-resolution time
		static uint64 Get_High_Resolution_Time( void );

		static double Convert_High_Resolution_Time_To_Seconds( uint64 ticks );
		static uint64 Convert_Seconds_To_High_Resolution_Ticks( double seconds );

		// File/OS time
		static uint64 Get_Raw_Time( void );
		static uint64 Get_File_Write_Raw_Time( const std::wstring &file_name );

		static std::wstring Format_Raw_Time( uint64 raw_time );

		static bool Is_Raw_Time_Less_Than_Seconds( uint64 time1, uint64 time2, uint64 seconds );
		static bool Is_Raw_Time_Greater_Than_Seconds( uint64 time1, uint64 time2, uint64 seconds );

	private:

		static uint64 HighResolutionFrequency;

		static bool Initialized;

};

#endif // PLATFORM_TIME_H
