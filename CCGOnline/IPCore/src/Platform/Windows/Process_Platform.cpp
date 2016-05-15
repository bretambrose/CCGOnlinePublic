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

#include <IPCore/Process/Process.h>

#include <IPCore/Utils/StringUtils.h>

#include <regex>

#include "Shlwapi.h"

namespace IP
{
namespace Process
{

uint32_t Get_Self_PID( void )
{
	return ::GetCurrentProcessId();
}


IP::String Get_Exe_Name( void )
{

	static char file_name_buffer[ 1024 ];
	::GetModuleFileName( 0, file_name_buffer, sizeof( file_name_buffer ) );

	std::regex _FilenamePattern(".*\\\\(\\w+)\\.(\\w*)$");	

	IP::String file_name(file_name_buffer);

	std::cmatch filename_match_results;
	std::regex_search(file_name.c_str(), filename_match_results, _FilenamePattern);

	IP::String exe_name = filename_match_results[1].str().c_str();

	return exe_name;
}


IP::String Get_Service_Name( void )
{
	IP::String exe_name = Get_Exe_Name();

	size_t name_length = exe_name.size();
	if ( name_length > 3 )
	{
		IP::String config_platform;
		IP::StringUtils::To_Upper_Case( exe_name.c_str() + name_length - 3, config_platform );
		
		if ( config_platform == "D32" || config_platform == "R32" || config_platform == "D64" || config_platform == "R64" )
		{
			return IP::String( exe_name.c_str(), exe_name.c_str() + name_length - 3 );
		}
	}

	return exe_name;
}

} // namespace Process
} // namespace IP
