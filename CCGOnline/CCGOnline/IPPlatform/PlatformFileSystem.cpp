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

#include "PlatformFileSystem.h"
#include "Shlwapi.h"


std::wstring NPlatform::Strip_Path( const std::wstring &full_path )
{
	return std::wstring( ::PathFindFileName( full_path.c_str() ) );
}


bool NPlatform::Directory_Exists( const std::wstring &path )
{
	DWORD attribs = ::GetFileAttributesW( path.c_str() );
	if ( attribs == INVALID_FILE_ATTRIBUTES ) 
	{
		return false;
	}

	return ( attribs & FILE_ATTRIBUTE_DIRECTORY ) != 0;
}


bool NPlatform::Create_Directory( const std::wstring &path )
{
	return ::CreateDirectoryW( path.c_str(), nullptr ) != 0;
} 


void NPlatform::Delete_Directory( const std::wstring &path )
{
	::RemoveDirectoryW( path.c_str() );
}


void NPlatform::Enumerate_Matching_Files( const std::wstring &pattern, std::vector< std::wstring > &file_names )
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
			file_names.push_back( std::wstring( find_info.cFileName ) );
		}
	}
	while ( ::FindNextFile( file_handle, &find_info ) != 0 );

	::FindClose( file_handle );
}


bool NPlatform::Delete_File( const std::wstring &file_name )
{
	return ::DeleteFile( file_name.c_str() ) != FALSE;
}


