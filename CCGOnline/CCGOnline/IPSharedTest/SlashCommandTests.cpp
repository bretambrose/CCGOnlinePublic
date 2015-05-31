/**********************************************************************************************************************

	SlashCommandTests.cpp
		defines unit tests for slash command related functionality

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

#include "IPShared/SlashCommands/SlashCommandManager.h"
#include "IPShared/SlashCommands/SlashCommandInstance.h"
#include "IPShared/Serialization/SerializationRegistrar.h"

class SlashCommandTests : public testing::Test 
{
	protected:  

		virtual void SetUp( void )
		{
			CSlashCommandManager::Initialize();
			CSerializationRegistrar::Finalize();
			CSlashCommandManager::Load_Command_File( "Data/XML/SlashCommandTests.xml" );
		}

		virtual void TearDown( void )
		{
			CSlashCommandManager::Shutdown();
		}

	private:

};

TEST_F( SlashCommandTests, Commands_Good )
{
	std::wstring error_msg;
	CSlashCommandInstance command_instance;
	ASSERT_TRUE( CSlashCommandManager::Parse_Command( L"/test1 5", command_instance, error_msg ) );

	ASSERT_TRUE( command_instance.Get_Param_Count() == 1 );

	uint32 value_uint32 = 0;
	ASSERT_TRUE( command_instance.Get_Param( 0, value_uint32 ) );
	ASSERT_TRUE( value_uint32 == 5 );

	command_instance.Reset();
	ASSERT_TRUE( CSlashCommandManager::Parse_Command( L"/test subtest \"5\" \"blah blah\"", command_instance, error_msg ) );

	ASSERT_TRUE( command_instance.Get_Param_Count() == 2 );

	int32 value_int32 = 0;
	ASSERT_TRUE( command_instance.Get_Param( 0, value_int32 ) );
	ASSERT_TRUE( value_int32 == 5 );

	std::string value_string;
	ASSERT_TRUE( command_instance.Get_Param( 1, value_string ) );
	ASSERT_TRUE( value_string == "blah blah" );

	command_instance.Reset();
	ASSERT_TRUE( CSlashCommandManager::Parse_Command( L"/test subby true 3.0 -5000", command_instance, error_msg ) );

	ASSERT_TRUE( command_instance.Get_Param_Count() == 3 );

	bool value_bool = false;
	ASSERT_TRUE( command_instance.Get_Param( 0, value_bool ) );
	ASSERT_TRUE( value_bool == true );

	double value_double = 0.0;
	ASSERT_TRUE( command_instance.Get_Param( 1, value_double ) );
	ASSERT_TRUE( value_double == 3.0 );

	int64 value_int64 = 0;
	ASSERT_TRUE( command_instance.Get_Param( 2, value_int64 ) );
	ASSERT_TRUE( value_int64 == -5000 );

	command_instance.Reset();
	ASSERT_TRUE( CSlashCommandManager::Parse_Command( L"/test subby true 4.0", command_instance, error_msg ) );

	ASSERT_TRUE( command_instance.Get_Param_Count() == 3 );

	value_bool = false;
	ASSERT_TRUE( command_instance.Get_Param( 0, value_bool ) );
	ASSERT_TRUE( value_bool == true );

	value_double = 0.0;
	ASSERT_TRUE( command_instance.Get_Param( 1, value_double ) );
	ASSERT_TRUE( value_double == 4.0 );

	value_int64 = 0;
	ASSERT_TRUE( command_instance.Get_Param( 2, value_int64 ) );
	ASSERT_TRUE( value_int64 == 666 );

	command_instance.Reset();
	ASSERT_TRUE( CSlashCommandManager::Parse_Command( L"/saytest Hello, world. Lalala.", command_instance, error_msg ) );

	ASSERT_TRUE( command_instance.Get_Param_Count() == 1 );

	std::wstring value_wstring;
	ASSERT_TRUE( command_instance.Get_Param( 0, value_wstring ) );
	ASSERT_TRUE( value_wstring == L"Hello, world. Lalala." );
}

TEST_F( SlashCommandTests, Commands_Bad )
{
	std::wstring error_msg;
	CSlashCommandInstance command_instance;

	ASSERT_FALSE( CSlashCommandManager::Parse_Command( L"/test1 ", command_instance, error_msg ) );
	ASSERT_FALSE( CSlashCommandManager::Parse_Command( L"/test1 badnumber", command_instance, error_msg ) );
	ASSERT_FALSE( CSlashCommandManager::Parse_Command( L"/test blah ", command_instance, error_msg ) );
	ASSERT_FALSE( CSlashCommandManager::Parse_Command( L"/test subtest 3", command_instance, error_msg ) );
	ASSERT_FALSE( CSlashCommandManager::Parse_Command( L"/test subby false", command_instance, error_msg ) );
	ASSERT_FALSE( CSlashCommandManager::Parse_Command( L"/test subby false notanumber", command_instance, error_msg ) );
}
