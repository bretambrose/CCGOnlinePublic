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

#include <sys/types.h>
#include <sys/stat.h>

namespace IP
{
namespace Time
{

std::tm Localtime(std::time_t time)
{
	std::tm tm_snapshot;
	localtime_r(&time, &tm_snapshot);

	return tm_snapshot;
}

SystemTimePoint Get_File_Last_Modified_Time( const IP::String &file_name )
{
	auto fp = fopen( file_name.c_str(), "r" );
	FATAL_ASSERT( fp != nullptr );

	int fd = fileno( fp );
	FATAL_ASSERT( fd != -1 );

	struct stat file_stats;
	auto result = fstat( fd, &file_stats );	// Windows-specific
	FATAL_ASSERT( result == 0 );

	fclose(fp);

	return std::chrono::system_clock::from_time_t(file_stats.st_mtime);
}
	
} // namespace Time
} // namespace IP

