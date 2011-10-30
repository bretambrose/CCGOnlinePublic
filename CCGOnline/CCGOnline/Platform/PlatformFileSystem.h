/**********************************************************************************************************************

	PlatformFileSystem.h
		A component that wraps miscellaneous OS-specific file logic

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

#ifndef PLATFORM_FILE_SYSTEM_H
#define PLATFORM_FILE_SYSTEM_H

namespace NPlatform
{
	// Directory functions
	bool Directory_Exists( const std::wstring &path );
	bool Create_Directory( const std::wstring &path );
	void Delete_Directory( const std::wstring &path );

	// File functions
	void Enumerate_Matching_Files( const std::wstring &pattern, std::vector< std::wstring > &file_names );

	bool Delete_File( const std::wstring &file_name );

	// Misc file-related
	std::wstring Strip_Path( const std::wstring &full_path );
}

#endif // PLATFORM_FILE_SYSTEM_H