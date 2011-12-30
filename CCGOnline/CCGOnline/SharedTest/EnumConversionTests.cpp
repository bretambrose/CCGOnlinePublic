/**********************************************************************************************************************

	EnumConversionTests.cpp
		defines unit tests for enum conversion related functionality

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

#include "EnumConversion.h"

TEST( EnumConversionTests, Normal )
{
	CEnumConverter::Register_Enum( "ETestEnum", CEP_NONE );

	CEnumConverter::Register_Enum_Entry( "ETestEnum", "Invalid", 0 );
	CEnumConverter::Register_Enum_Entry( "ETestEnum", "Entry1", 1 );
	CEnumConverter::Register_Enum_Entry( "ETestEnum", "Entry2", 2 );
	CEnumConverter::Register_Enum_Entry( "ETestEnum", "Entry3", 3 );

	uint64 converted_value = 0;
	ASSERT_TRUE( CEnumConverter::Convert( "ETEStEnuM", "Entry1", converted_value ) );
	ASSERT_TRUE( converted_value == 1 );

	ASSERT_TRUE( CEnumConverter::Convert( "ETESTENUM", "eNTRy2", converted_value ) );
	ASSERT_TRUE( converted_value == 2 );

	ASSERT_FALSE( CEnumConverter::Convert( "ETEStEnu", "Entry1", converted_value ) );
	ASSERT_FALSE( CEnumConverter::Convert( "ETEStEnuM", "Entryy1", converted_value ) );

	std::string converted_entry;
	ASSERT_TRUE( CEnumConverter::Convert( "ETEStEnuM", 3, converted_entry ) );
	ASSERT_TRUE( converted_entry == "ENTRY3" );

	ASSERT_TRUE( CEnumConverter::Convert( "ETESTENUM", 0, converted_entry ) );
	ASSERT_TRUE( converted_entry == "INVALID" );

	ASSERT_FALSE( CEnumConverter::Convert( "ETEStEnu", 3, converted_entry ) );
	ASSERT_FALSE( CEnumConverter::Convert( "ETEStEnuM", 10, converted_entry ) );
}

TEST( EnumConversionTests, Bitfield )
{
	CEnumConverter::Register_Enum( "ETestBitfield", CEP_BITFIELD );

	CEnumConverter::Register_Enum_Entry( "ETestBitfield", "None", 0 );
	CEnumConverter::Register_Enum_Entry( "ETestBitfield", "Flag1", 1 );
	CEnumConverter::Register_Enum_Entry( "ETestBitfield", "Flag2", 2 );
	CEnumConverter::Register_Enum_Entry( "ETestBitfield", "Flag3", 4 );
	CEnumConverter::Register_Enum_Entry( "ETestBitfield", "Flag4", 8 );
	CEnumConverter::Register_Enum_Entry( "ETestBitfield", "Flag5", 1ULL << 36 );

	uint64 converted_value = 0;
	ASSERT_TRUE( CEnumConverter::Convert( "ETestBITfield", "NoNe", converted_value ) );
	ASSERT_TRUE( converted_value == 0 );

	ASSERT_TRUE( CEnumConverter::Convert( "ETestBITfiELd", "FLag3", converted_value ) );
	ASSERT_TRUE( converted_value == 4 );

	ASSERT_TRUE( CEnumConverter::Convert( "ETestBITfiELd", "FLag3|Flag1", converted_value ) );
	ASSERT_TRUE( converted_value == 5 );

	ASSERT_TRUE( CEnumConverter::Convert( "ETestBITfiELd", "FLag2| Flag4", converted_value ) );
	ASSERT_TRUE( converted_value == 10 );

	ASSERT_TRUE( CEnumConverter::Convert( "ETestBITfiELd", "FLag2| Flag4|FLAG1 |FlaG3", converted_value ) );
	ASSERT_TRUE( converted_value == 15 );

	ASSERT_TRUE( CEnumConverter::Convert( "ETestBITfiELd", "FLag1| Flag5", converted_value ) );
	ASSERT_TRUE( converted_value == (1ULL << 36) + 1 );

	ASSERT_FALSE( CEnumConverter::Convert( "ETestBITfiel", "Flag1", converted_value ) );
	ASSERT_FALSE( CEnumConverter::Convert( "ETestBITfield", "Flagg1", converted_value ) );

	std::string converted_entry;
	ASSERT_TRUE( CEnumConverter::Convert( "ETestBITfield", 3, converted_entry ) );
	ASSERT_TRUE( converted_entry == "FLAG1 | FLAG2" );

	ASSERT_TRUE( CEnumConverter::Convert( "ETestBITfield", 0, converted_entry ) );
	ASSERT_TRUE( converted_entry == "NONE" );

	ASSERT_TRUE( CEnumConverter::Convert( "ETestBITfield", 15, converted_entry ) );
	ASSERT_TRUE( converted_entry == "FLAG1 | FLAG2 | FLAG3 | FLAG4" );

	ASSERT_FALSE( CEnumConverter::Convert( "ETestBITfiel", 3, converted_entry ) );
	ASSERT_FALSE( CEnumConverter::Convert( "ETestBITfield", 16, converted_entry ) );
}
