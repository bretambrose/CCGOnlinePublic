/**********************************************************************************************************************

	[Placeholder for eventual source license]

	PlatformFileSystem.h
		A component that wraps miscellaneous OS-specific file logic

	(c) Copyright 2011, Bret Ambrose.  All rights reserved.

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