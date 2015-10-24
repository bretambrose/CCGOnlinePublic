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

#include "stdafx.h"

#include "PlatformTime.h"
#include "StringUtils.h"

#include <sstream>
#include <iostream>
#include <iomanip>

#include <sys/types.h>
#include <sys/stat.h>

/*
uint64_t CPlatformTime::HighResolutionFrequency = 0;
bool CPlatformTime::Initialized = false;
*/

/*
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


uint64_t CPlatformTime::Get_High_Resolution_Time( void )
{
  LARGE_INTEGER count;
  ::QueryPerformanceCounter( &count );

  return count.QuadPart;
}


double CPlatformTime::Convert_High_Resolution_Time_To_Seconds( uint64_t ticks )
{
	FATAL_ASSERT( Initialized );

	return static_cast< double >( ticks ) / static_cast< double >( HighResolutionFrequency );
}


uint64_t CPlatformTime::Convert_Seconds_To_High_Resolution_Ticks( double seconds )
{
	FATAL_ASSERT( Initialized );

	return static_cast< uint64_t >( seconds *  static_cast< double >( HighResolutionFrequency ) );
}


uint64_t CPlatformTime::Get_Raw_Time( void )
{
	FILETIME file_time;
	::GetSystemTimeAsFileTime( &file_time );

	return static_cast< uint64_t >( file_time.dwLowDateTime ) | ( static_cast< uint64_t >( file_time.dwHighDateTime ) << 32 );
}


uint64_t CPlatformTime::Get_File_Write_Raw_Time( const std::wstring &file_name )
{
	FILETIME write_time;
	HANDLE file_handle = ::CreateFileW( file_name.c_str(), GENERIC_READ, FILE_SHARE_READ, nullptr,  OPEN_EXISTING, 0, nullptr );
	FATAL_ASSERT( file_handle != INVALID_HANDLE_VALUE );

	::GetFileTime( file_handle, nullptr, nullptr, &write_time );

	::CloseHandle( file_handle );

	return static_cast< uint64_t >( write_time.dwLowDateTime ) | ( static_cast< uint64_t >( write_time.dwHighDateTime ) << 32 );
}


bool CPlatformTime::Is_Raw_Time_Less_Than_Seconds( uint64_t time1, uint64_t time2, uint64_t seconds )
{
	static const uint64_t HUNDRED_NANOSECONDS_IN_A_SECOND = 10000000;

	return time1 + seconds * HUNDRED_NANOSECONDS_IN_A_SECOND < time2;
}


bool CPlatformTime::Is_Raw_Time_Greater_Than_Seconds( uint64_t time1, uint64_t time2, uint64_t seconds )
{
	static const uint64_t HUNDRED_NANOSECONDS_IN_A_SECOND = 10000000;

	return time1 + seconds * HUNDRED_NANOSECONDS_IN_A_SECOND > time2;
}


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
*/
// v2.0 interface

namespace IP
{
namespace Time
{

SystemTimePoint Get_Current_System_Time( void )
{
	return std::chrono::system_clock::now();
}

SystemTimePoint Get_File_Last_Modified_Time( const std::wstring &file_name )
{
	std::string narrow_file_name;
	IP::String::WideString_To_String( file_name, narrow_file_name );

	FILE *fp = nullptr;
	auto open_result = fopen_s( &fp, narrow_file_name.c_str(), "r" );
	FATAL_ASSERT( fp != nullptr && open_result == 0 );

	int fd = _fileno( fp ); 
	FATAL_ASSERT( fd != -1 );

	struct _stat file_stats;
	auto result = _fstat( fd, &file_stats );	// Windows-specific
	FATAL_ASSERT( result == 0 );

	fclose(fp);

	return std::chrono::system_clock::from_time_t(file_stats.st_mtime);
}

/*
void CTimeToFileTime(time_t t, FILETIME &windows_file_time)
{
	int64_t ll;

	ll = Int32x32To64(t, 10000000) + 116444736000000000;
	windows_file_time.dwLowDateTime = (DWORD)ll;
	windows_file_time.dwHighDateTime = ll >> 32;
}

void CTimeToSystemTime(time_t t, SYSTEMTIME &windows_system_time)
{
	FILETIME file_time;

	CTimeToFileTime(t, file_time);
	FileTimeToSystemTime(&file_time, &windows_system_time);
}
*/

std::tm localtime(std::time_t time)
{
	std::tm tm_snapshot;
#if (defined(WIN32) || defined(_WIN32) || defined(__WIN32__))
	localtime_s(&tm_snapshot, &time); 
#else
	localtime_r(&time, &tm_snapshot); // POSIX  
#endif
	return tm_snapshot;
}

std::wstring Format_System_Time( SystemTimePoint time_point )
{
	auto c_time = std::chrono::system_clock::to_time_t( time_point );
	auto tm_time = localtime( c_time );
	auto year_remainder = tm_time.tm_year % 100;

	// Grungy hack assumes that system clock "starts" with 0 milliseconds elapsed
	auto milliseconds_elapsed = std::chrono::duration_cast< std::chrono::milliseconds >( time_point - SystemTimePoint::min() );
	auto milliseconds_remainder = milliseconds_elapsed.count() % 1000;

	std::basic_ostringstream< wchar_t > output_string;
	output_string << std::setw( 2 ) << tm_time.tm_mon << L"-" << tm_time.tm_mday << L"-" << year_remainder << L" ";
	output_string << std::setw( 2 ) << tm_time.tm_hour << L":" << tm_time.tm_min << L":" << tm_time.tm_sec << L"." << std::setw( 3 ) << milliseconds_remainder;

	return output_string.rdbuf()->str();
}

SystemDuration Get_Elapsed_System_Time( void )
{
	auto now = std::chrono::system_clock::now();

	return now - SystemTimePoint::min();
}

double Convert_Duration_To_Seconds(SystemDuration duration)
{
	// Question: what is the best way of doing this precision-wise?
	double raw_count = static_cast< double >( duration.count() );

	return raw_count / static_cast< double >( SystemDuration::period::den ) * static_cast< double >( SystemDuration::period::num );
}

} // namespace Time
} // namespace IP

