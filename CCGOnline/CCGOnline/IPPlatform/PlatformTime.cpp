/**********************************************************************************************************************

	PlatformTime.cpp
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

#include "stdafx.h"

#include "PlatformTime.h"

#include <sstream>
#include <iostream>
#include <iomanip>

uint64_t CPlatformTime::HighResolutionFrequency = 0;
bool CPlatformTime::Initialized = false;

#ifdef WIN32

/**********************************************************************************************************************
	CPlatformTime::Initialize -- initializes the class by querying performance counter information

**********************************************************************************************************************/
void CPlatformTime::Initialize( void )
{
	if ( Initialized )
	{
		return;
	}

	LARGE_INTEGER frequency;
	::QueryPerformanceFrequency( &frequency );

	HighResolutionFrequency = frequency.QuadPart;

	FATAL_ASSERT( HighResolutionFrequency != 0 );

	Initialized = true;
}

/**********************************************************************************************************************
	CPlatformTime::Get_High_Resolution_Time -- gets the current time using the highest resolution timer the os
		has available

		Returns: the current time in high-resolution units

**********************************************************************************************************************/
uint64_t CPlatformTime::Get_High_Resolution_Time( void )
{
  LARGE_INTEGER count;
  ::QueryPerformanceCounter( &count );

  return count.QuadPart;
}

/**********************************************************************************************************************
	CPlatformTime::Convert_High_Resolution_Time_To_Seconds -- converts a high-resolution time to seconds

		ticks -- a high resolution time value

		Returns: time in seconds the supplied tick count represents

**********************************************************************************************************************/
double CPlatformTime::Convert_High_Resolution_Time_To_Seconds( uint64_t ticks )
{
	FATAL_ASSERT( Initialized );

	return static_cast< double >( ticks ) / static_cast< double >( HighResolutionFrequency );
}

/**********************************************************************************************************************
	CPlatformTime::Convert_Seconds_To_High_Resolution_Ticks -- converts a time in seconds to high-res ticks

		seconds -- a time in seconds

		Returns: high-resolution ticks represented by that supplied time value

**********************************************************************************************************************/
uint64_t CPlatformTime::Convert_Seconds_To_High_Resolution_Ticks( double seconds )
{
	FATAL_ASSERT( Initialized );

	return static_cast< uint64_t >( seconds *  static_cast< double >( HighResolutionFrequency ) );
}

/**********************************************************************************************************************
	CPlatformTime::Get_Raw_Time -- gets the current time using the OS's generic standard system query

		Returns: the current time in os-specific units

**********************************************************************************************************************/
uint64_t CPlatformTime::Get_Raw_Time( void )
{
	FILETIME file_time;
	::GetSystemTimeAsFileTime( &file_time );

	return static_cast< uint64_t >( file_time.dwLowDateTime ) | ( static_cast< uint64_t >( file_time.dwHighDateTime ) << 32 );
}

/**********************************************************************************************************************
	CPlatformTime::Get_File_Write_Raw_Time -- gets the last-modified time for a file, in generic os time units

		file_name -- file to get the last written time for

		Returns: the last-written time of the file in os-specific units

**********************************************************************************************************************/
uint64_t CPlatformTime::Get_File_Write_Raw_Time( const std::wstring &file_name )
{
	FILETIME write_time;
	HANDLE file_handle = ::CreateFileW( file_name.c_str(), GENERIC_READ, FILE_SHARE_READ, nullptr,  OPEN_EXISTING, 0, nullptr );
	FATAL_ASSERT( file_handle != INVALID_HANDLE_VALUE );

	::GetFileTime( file_handle, nullptr, nullptr, &write_time );

	::CloseHandle( file_handle );

	return static_cast< uint64_t >( write_time.dwLowDateTime ) | ( static_cast< uint64_t >( write_time.dwHighDateTime ) << 32 );
}

/**********************************************************************************************************************
	CPlatformTime::Is_Raw_Time_Less_Than_Seconds -- compares two os-specific system time values

		time1 -- an os-specific time value
		time2 -- an os-specific time value
		seconds -- a time value in seconds

		Returns: true if time1_in_seconds + seconds < time2_in_seconds

**********************************************************************************************************************/
bool CPlatformTime::Is_Raw_Time_Less_Than_Seconds( uint64_t time1, uint64_t time2, uint64_t seconds )
{
	static const uint64_t HUNDRED_NANOSECONDS_IN_A_SECOND = 10000000;

	return time1 + seconds * HUNDRED_NANOSECONDS_IN_A_SECOND < time2;
}

/**********************************************************************************************************************
	CPlatformTime::Is_Raw_Time_Greater_Than_Seconds -- compares two os-specific system time values

		time1 -- an os-specific time value
		time2 -- an os-specific time value
		seconds -- a time value in seconds

		Returns: true if time1_in_seconds + seconds > time2_in_seconds

**********************************************************************************************************************/
bool CPlatformTime::Is_Raw_Time_Greater_Than_Seconds( uint64_t time1, uint64_t time2, uint64_t seconds )
{
	static const uint64_t HUNDRED_NANOSECONDS_IN_A_SECOND = 10000000;

	return time1 + seconds * HUNDRED_NANOSECONDS_IN_A_SECOND > time2;
}

/**********************************************************************************************************************
	CPlatformTime::Format_Raw_Time -- takes an os-specific time value and converts it to a human-readable string

		raw_time -- os-specific time value

		Returns: formatted time string

**********************************************************************************************************************/
std::wstring CPlatformTime::Format_Raw_Time( uint64_t raw_time )
{
	FILETIME file_time;
	file_time.dwHighDateTime = static_cast< uint32_t >( ( raw_time >> 32 ) & 0xFFFFFFFF );
	file_time.dwLowDateTime = static_cast< uint32_t >( raw_time & 0xFFFFFFFF );

	SYSTEMTIME system_time;
	::FileTimeToSystemTime( &file_time, &system_time );

	std::basic_ostringstream< wchar_t > output_string;
	output_string << std::setw( 2 ) << system_time.wHour << L":" << system_time.wMinute << L":" << system_time.wSecond << L":" << std::setw( 4 ) << system_time.wMilliseconds;

	return output_string.rdbuf()->str();
}

#endif // WIN32
