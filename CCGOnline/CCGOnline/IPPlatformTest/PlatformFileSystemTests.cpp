/**********************************************************************************************************************

	PlatformFileSystemTests.cpp
		defines unit tests for file-system-related platform-specific functionality

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

#include <fstream>
#include <iostream>

#include "IPPlatform/PlatformFileSystem.h"

static const std::wstring TEST_DIRECTORY( L"DirectoryTest" );
static const std::wstring TEST_FILE1( L"FileTest1.txt" );
static const std::wstring TEST_FILE2( L"FileTest2.txt" );
static const std::wstring TEST_FILE_PATTERN( L"FileTest*.txt" );

class PlatformFileSystemTests : public testing::Test 
{
	protected:  

		static void SetUpTestCase( void ) 
		{			
			Destroy_Test_Directory();
			Destroy_Test_Files();
		}

		static void TearDownTestCase( void )
		{
			Destroy_Test_Directory();
			Destroy_Test_Files();
		}

	private:

		static void Destroy_Test_Directory( void )
		{
			if ( NPlatform::Directory_Exists( TEST_DIRECTORY ) )
			{
				NPlatform::Delete_Directory( TEST_DIRECTORY );
			}
		}

		static void Destroy_Test_Files( void )
		{
			NPlatform::Delete_File( TEST_FILE1 );
			NPlatform::Delete_File( TEST_FILE2 );
		}

};

TEST_F( PlatformFileSystemTests, Directory_Ops )
{
	ASSERT_FALSE( NPlatform::Directory_Exists( std::wstring( L"GarbageDirectory" ) ) );
	ASSERT_TRUE( NPlatform::Directory_Exists( std::wstring( L"Data" ) ) );
	ASSERT_TRUE( NPlatform::Directory_Exists( std::wstring( L"x86\\External_DLLs" ) ) );
	ASSERT_TRUE( NPlatform::Directory_Exists( std::wstring( L"x64\\External_DLLs" ) ) );

	ASSERT_FALSE( NPlatform::Directory_Exists( TEST_DIRECTORY ) );
	NPlatform::Create_Directory( TEST_DIRECTORY );
	ASSERT_TRUE( NPlatform::Directory_Exists( TEST_DIRECTORY ) );
	NPlatform::Delete_Directory( TEST_DIRECTORY );
	ASSERT_FALSE( NPlatform::Directory_Exists( TEST_DIRECTORY ) );
}

TEST_F( PlatformFileSystemTests, File_Ops )
{
	std::vector< std::wstring > file_names;
	NPlatform::Enumerate_Matching_Files( TEST_FILE_PATTERN, file_names );

	ASSERT_TRUE( file_names.size() == 0 );

	std::basic_ofstream< wchar_t > file1( TEST_FILE1.c_str(), std::ios_base::out | std::ios_base::trunc );
	file1 << L"test1\n";
	file1.close();

	std::basic_ofstream< wchar_t > file2( TEST_FILE2.c_str(), std::ios_base::out | std::ios_base::trunc );
	file2 << L"test2\n";
	file2.close();

	NPlatform::Enumerate_Matching_Files( TEST_FILE_PATTERN, file_names );
	ASSERT_TRUE( file_names.size() == 2 );

	for ( uint32 i = 0; i < file_names.size(); i++ )
	{
		NPlatform::Delete_File( file_names[ i ] );
	}

	file_names.clear();

	NPlatform::Enumerate_Matching_Files( TEST_FILE_PATTERN, file_names );
	ASSERT_TRUE( file_names.size() == 0 );
}

TEST_F( PlatformFileSystemTests, Strip_Path )
{
	static const std::wstring test_file_name( L"TestFileName.exe" );
	static const std::wstring path1( test_file_name );
	static const std::wstring path2 = std::wstring( L"../SomeDirectory/" ) + test_file_name;
	static const std::wstring path3 = std::wstring( L"C:\\SomeDir\\SomeOtherDir\\" ) + test_file_name;
	
	ASSERT_TRUE( NPlatform::Strip_Path( path1 ) == test_file_name );
	ASSERT_TRUE( NPlatform::Strip_Path( path2 ) == test_file_name );
	ASSERT_TRUE( NPlatform::Strip_Path( path3 ) == test_file_name );
}

