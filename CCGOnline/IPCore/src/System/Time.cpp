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

#include <IPCore/System/Time.h>

#include <IPCore/Debug/DebugAssert.h>
#include <IPCore/Memory/Stl/StringStream.h>
#include <IPCore/Utils/StringUtils.h>

#include <iostream>
#include <iomanip>

namespace IP
{
namespace Time
{

SystemTimePoint Get_Current_System_Time( void )
{
	return std::chrono::system_clock::now();
}

IP::String Format_System_Time( SystemTimePoint time_point )
{
	auto c_time = std::chrono::system_clock::to_time_t( time_point );
	auto tm_time = IP::Time::Localtime( c_time );
	auto year_remainder = tm_time.tm_year % 100;

	// Grungy hack assumes that system clock "starts" with 0 milliseconds elapsed
	auto milliseconds_elapsed = std::chrono::duration_cast< std::chrono::milliseconds >( time_point - SystemTimePoint::min() );
	auto milliseconds_remainder = milliseconds_elapsed.count() % 1000;

	IP::StringStream output_string;
	output_string << std::setw( 2 ) << tm_time.tm_mon << "-" << tm_time.tm_mday << "-" << year_remainder << " ";
	output_string << std::setw( 2 ) << tm_time.tm_hour << ":" << tm_time.tm_min << ":" << tm_time.tm_sec << "." << std::setw( 3 ) << milliseconds_remainder;

	return output_string.str();
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

