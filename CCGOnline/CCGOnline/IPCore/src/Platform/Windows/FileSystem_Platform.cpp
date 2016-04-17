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

#include <IPCore/File/FileSystem.h>

#include "Shlwapi.h"

namespace IP
{
namespace FileSystem
{

IP::String Strip_Path( const IP::String &full_path )
{
	return IP::String( ::PathFindFileName( full_path.c_str() ) );
}


bool Directory_Exists( const IP::String &path )
{
	DWORD attribs = ::GetFileAttributes( path.c_str() );
	if ( attribs == INVALID_FILE_ATTRIBUTES ) 
	{
		return false;
	}

	return ( attribs & FILE_ATTRIBUTE_DIRECTORY ) != 0;
}


bool Create_Directory( const IP::String &path )
{
	return ::CreateDirectory( path.c_str(), nullptr ) != 0;
} 


void Delete_Directory( const IP::String &path )
{
	::RemoveDirectory( path.c_str() );
}


void Enumerate_Matching_Files( const IP::String &pattern, IP::Vector< IP::String > &file_names )
{
	file_names.clear();

	HANDLE file_handle = INVALID_HANDLE_VALUE;
	WIN32_FIND_DATA find_info;

	file_handle = ::FindFirstFile( pattern.c_str(), &find_info );
	if ( INVALID_HANDLE_VALUE == file_handle ) 
	{
		return;
	} 
   
	do
	{
		if ( ( find_info.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY ) == 0 )
		{
			file_names.push_back( IP::String( find_info.cFileName ) );
		}
	}
	while ( ::FindNextFile( file_handle, &find_info ) != 0 );

	::FindClose( file_handle );
}


bool Delete_File( const IP::String &file_name )
{
	if ( DeleteFile( file_name.c_str() ) == 0 )
	{
		DWORD error = GetLastError();
		error = error;
		return false;
	}

	return true;
}

} // namespace FileSystem
} // namespace IP

