/**********************************************************************************************************************

	PlatformMisc.cpp
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

#include "stdafx.h"

#include "PlatformProcess.h"

#include <regex>
#include "Shlwapi.h"
#include "StringUtils.h"

/**********************************************************************************************************************
	NPlatform::Get_Self_Process_ID -- gets the id of the current process

		Returns: a 32 bit process id

**********************************************************************************************************************/
uint32_t NPlatform::Get_Self_PID( void )
{
	return ::GetCurrentProcessId();
}

/**********************************************************************************************************************
	NPlatform::Get_Exe_Name -- gets the file name of the current executing process

		Returns: the file name of the executing process, minus any . extension

**********************************************************************************************************************/
std::wstring NPlatform::Get_Exe_Name( void )
{

	static wchar_t file_name_buffer[ 1024 ];
	::GetModuleFileName( 0, file_name_buffer, sizeof( file_name_buffer ) );

	static std::tr1::wregex _FilenamePattern(L".*\\\\(\\w+)\\.(\\w*)$");	

	std::wstring file_name(file_name_buffer);

	std::tr1::wcmatch filename_match_results;
	std::tr1::regex_search(file_name.c_str(), filename_match_results, _FilenamePattern);

	std::wstring exe_name = filename_match_results[1];

	return exe_name;
}

/**********************************************************************************************************************
	NPlatform::Get_Service_Name -- gets the service name of the current executing process.  The service name is the
		file name minus the extension and minus release/debug (R/D) and 32/64 bit tags.

		Returns: the service name of the executing process

**********************************************************************************************************************/
std::wstring NPlatform::Get_Service_Name( void )
{
	std::wstring exe_name = Get_Exe_Name();

	size_t name_length = exe_name.size();
	if ( name_length > 3 )
	{
		std::wstring config_platform;
		NStringUtils::To_Upper_Case( exe_name.c_str() + name_length - 3, config_platform );
		
		if ( config_platform == L"D32" || config_platform == L"R32" || config_platform == L"D64" || config_platform == L"R64" )
		{
			return std::wstring( exe_name.c_str(), exe_name.c_str() + name_length - 3 );
		}
	}

	return exe_name;
}

/**********************************************************************************************************************
	NPlatform::Sleep -- asks the os to put the current thread to sleep for a period of time

		milliseconds -- time to sleep for, in milliseconds

**********************************************************************************************************************/
void NPlatform::Sleep( uint32_t milliseconds )
{
	::Sleep( milliseconds );
}