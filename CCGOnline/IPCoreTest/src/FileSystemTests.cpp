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

#include <gtest/gtest.h>

#include <fstream>
#include <iostream>


static const IP::String TEST_DIRECTORY( "DirectoryTest" );
static const IP::String TEST_FILE1( "FileTest1.txt" );
static const IP::String TEST_FILE2( "FileTest2.txt" );
static const IP::String TEST_FILE_PATTERN( "FileTest*.txt" );

class FileSystemTests : public testing::Test 
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
			if ( IP::FileSystem::Directory_Exists( TEST_DIRECTORY ) )
			{
				IP::FileSystem::Delete_Directory( TEST_DIRECTORY );
			}
		}

		static void Destroy_Test_Files( void )
		{
			IP::FileSystem::Delete_File( TEST_FILE1 );
			IP::FileSystem::Delete_File( TEST_FILE2 );
		}

};

TEST_F( FileSystemTests, Directory_Ops )
{
	ASSERT_FALSE( IP::FileSystem::Directory_Exists( TEST_DIRECTORY ) );
	IP::FileSystem::Create_Directory( TEST_DIRECTORY );
	ASSERT_TRUE( IP::FileSystem::Directory_Exists( TEST_DIRECTORY ) );
	IP::FileSystem::Delete_Directory( TEST_DIRECTORY );
	ASSERT_FALSE( IP::FileSystem::Directory_Exists( TEST_DIRECTORY ) );
}

TEST_F( FileSystemTests, File_Ops )
{
	IP::Vector< IP::String > file_names;
	IP::FileSystem::Enumerate_Matching_Files( TEST_FILE_PATTERN, file_names );

	ASSERT_TRUE( file_names.size() == 0 );

	std::basic_ofstream< char > file1( TEST_FILE1.c_str(), std::ios_base::out | std::ios_base::trunc );
	file1 << "test1\n";
	file1.close();

	std::basic_ofstream< char > file2( TEST_FILE2.c_str(), std::ios_base::out | std::ios_base::trunc );
	file2 << "test2\n";
	file2.close();

	IP::FileSystem::Enumerate_Matching_Files( TEST_FILE_PATTERN, file_names );
	ASSERT_TRUE( file_names.size() == 2 );

	for ( uint32_t i = 0; i < file_names.size(); i++ )
	{
		IP::FileSystem::Delete_File( file_names[ i ] );
	}

	file_names.clear();

	IP::FileSystem::Enumerate_Matching_Files( TEST_FILE_PATTERN, file_names );
	ASSERT_TRUE( file_names.size() == 0 );
}

TEST_F( FileSystemTests, Strip_Path )
{
	static const IP::String test_file_name( "TestFileName.exe" );
	static const IP::String path1( test_file_name );
	static const IP::String path2 = IP::String( "../SomeDirectory/" ) + test_file_name;
	static const IP::String path3 = IP::String( "C:\\SomeDir\\SomeOtherDir\\" ) + test_file_name;
	
	ASSERT_TRUE( IP::FileSystem::Strip_Path( path1 ) == test_file_name );
	ASSERT_TRUE( IP::FileSystem::Strip_Path( path2 ) == test_file_name );
	ASSERT_TRUE( IP::FileSystem::Strip_Path( path3 ) == test_file_name );
}

