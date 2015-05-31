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

#include "IPShared/EnumConversion.h"

static enum ETestEnum
{
	TE_INVALID,
	TE_ENTRY1,
	TE_ENTRY2, 
	TE_ENTRY3
};

TEST( EnumConversionTests, Normal )
{
	CEnumConverter::Register_Enum< ETestEnum >( "ETestEnum", CEP_NONE );

	CEnumConverter::Register_Enum_Entry( "Invalid", TE_INVALID );
	CEnumConverter::Register_Enum_Entry( "Entry1", TE_ENTRY1 );
	CEnumConverter::Register_Enum_Entry( "Entry2", TE_ENTRY2 );
	CEnumConverter::Register_Enum_Entry( "Entry3", TE_ENTRY3 );

	ETestEnum converted_value = TE_INVALID;
	ASSERT_TRUE( CEnumConverter::Convert( "Entry1", converted_value ) );
	ASSERT_TRUE( converted_value == TE_ENTRY1 );

	ASSERT_TRUE( CEnumConverter::Convert( "eNTRy2", converted_value ) );
	ASSERT_TRUE( converted_value == TE_ENTRY2 );

	ASSERT_FALSE( CEnumConverter::Convert( "Entryy1", converted_value ) );

	std::string converted_entry;
	ASSERT_TRUE( CEnumConverter::Convert( TE_ENTRY3, converted_entry ) );
	ASSERT_TRUE( converted_entry == "ENTRY3" );

	ASSERT_TRUE( CEnumConverter::Convert( TE_INVALID, converted_entry ) );
	ASSERT_TRUE( converted_entry == "INVALID" );

	ASSERT_FALSE( CEnumConverter::Convert( static_cast< ETestEnum >( 10 ), converted_entry ) );
}

enum ETestBitfield
{
	TB_NONE = 0
};

TEST( EnumConversionTests, Bitfield )
{
	CEnumConverter::Register_Enum< ETestBitfield >( "ETestBitfield", CEP_BITFIELD );

	CEnumConverter::Register_Enum_Entry( "None", static_cast< ETestBitfield >( 0 ) );
	CEnumConverter::Register_Enum_Entry( "Flag1", static_cast< ETestBitfield >( 1 ) );
	CEnumConverter::Register_Enum_Entry( "Flag2", static_cast< ETestBitfield >( 2 ) );
	CEnumConverter::Register_Enum_Entry( "Flag3", static_cast< ETestBitfield >( 4 ) );
	CEnumConverter::Register_Enum_Entry( "Flag4", static_cast< ETestBitfield >( 8 ) );

	ETestBitfield converted_value = TB_NONE;
	ASSERT_TRUE( CEnumConverter::Convert( "NoNe", converted_value ) );
	ASSERT_TRUE( converted_value == TB_NONE );

	ASSERT_TRUE( CEnumConverter::Convert( "FLag3", converted_value ) );
	ASSERT_TRUE( converted_value == 4 );

	ASSERT_TRUE( CEnumConverter::Convert( "FLag3|Flag1", converted_value ) );
	ASSERT_TRUE( converted_value == 5 );

	ASSERT_TRUE( CEnumConverter::Convert( "FLag2| Flag4", converted_value ) );
	ASSERT_TRUE( converted_value == 10 );

	ASSERT_TRUE( CEnumConverter::Convert( "FLag2| Flag4|FLAG1 |FlaG3", converted_value ) );
	ASSERT_TRUE( converted_value == 15 );

	ASSERT_FALSE( CEnumConverter::Convert( "Flagg1", converted_value ) );

	std::string converted_entry;
	ASSERT_TRUE( CEnumConverter::Convert( static_cast< ETestBitfield >( 3 ), converted_entry ) );
	ASSERT_TRUE( converted_entry == "FLAG1 | FLAG2" );

	ASSERT_TRUE( CEnumConverter::Convert( static_cast< ETestBitfield >( 0 ), converted_entry ) );
	ASSERT_TRUE( converted_entry == "NONE" );

	ASSERT_TRUE( CEnumConverter::Convert( static_cast< ETestBitfield >( 15 ), converted_entry ) );
	ASSERT_TRUE( converted_entry == "FLAG1 | FLAG2 | FLAG3 | FLAG4" );

	ASSERT_FALSE( CEnumConverter::Convert( static_cast< ETestBitfield >( 16 ), converted_entry ) );
}
