/**********************************************************************************************************************

	ReflectionTests.cpp
		defines unit tests for enum reflection

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

#include "ReflectionTests.h"
#include "EnumConversion.h"

TEST( ReflectionTests, Normal )
{
	EReflectionTest converted_value = RT_INVALID;
	ASSERT_TRUE( CEnumConverter::Convert( "EReflectionTest", "Entry1", converted_value ) );
	ASSERT_TRUE( converted_value == RT_ENTRY1 );

	ASSERT_TRUE( CEnumConverter::Convert( "EReflectionTest", "eNTRy2", converted_value ) );
	ASSERT_TRUE( converted_value == RT_ENTRY2 );

	ASSERT_TRUE( CEnumConverter::Convert( "EReflectionTest", "eNTRy3", converted_value ) );
	ASSERT_TRUE( converted_value == RT_ENTRY3 );

	ASSERT_TRUE( CEnumConverter::Convert( "EReflectionTest", "eNTRy4", converted_value ) );
	ASSERT_TRUE( converted_value == RT_ENTRY4 );

	ASSERT_FALSE( CEnumConverter::Convert( "EReflectionTes", "Entry1", converted_value ) );
	ASSERT_FALSE( CEnumConverter::Convert( "EReflectionTest", "Entryy1", converted_value ) );

	std::string converted_entry;
	ASSERT_TRUE( CEnumConverter::Convert( "EReflectionTest", RT_ENTRY2, converted_entry ) );
	ASSERT_TRUE( converted_entry == "ENTRY2" );

	ASSERT_TRUE( CEnumConverter::Convert( "EReflectionTest", RT_INVALID, converted_entry ) );
	ASSERT_TRUE( converted_entry == "INVALID" );

	ASSERT_FALSE( CEnumConverter::Convert( "EReflectionTes", 3, converted_entry ) );
	ASSERT_FALSE( CEnumConverter::Convert( "EReflectionTest", 10, converted_entry ) );
}

TEST( ReflectionTests, Bitfield )
{
	EReflectionBitfieldTest converted_value = RBT_NONE;
	ASSERT_TRUE( CEnumConverter::Convert( "EReflectionBitfieldTest", "NoNe", converted_value ) );
	ASSERT_TRUE( converted_value == RBT_NONE );

	ASSERT_TRUE( CEnumConverter::Convert( "EReflectionBitfieldTest", "Bit3", converted_value ) );
	ASSERT_TRUE( converted_value == 4 );

	ASSERT_TRUE( CEnumConverter::Convert( "EReflectionBitfieldTest", "BIt3|Bit1", converted_value ) );
	ASSERT_TRUE( converted_value == 5 );

	ASSERT_TRUE( CEnumConverter::Convert( "EReflectionBitfieldTest", "Bit2| BiT8", converted_value ) );
	ASSERT_TRUE( converted_value == 130 );

	ASSERT_TRUE( CEnumConverter::Convert( "EReflectionBitfieldTest", "BIT1| Bit2|Bit3 |Bit8", converted_value ) );
	ASSERT_TRUE( converted_value == 135 );

	ASSERT_FALSE( CEnumConverter::Convert( "EReflectionBitfieldTes", "Bit1", converted_value ) );
	ASSERT_FALSE( CEnumConverter::Convert( "EReflectionBitfieldTest", "Bitt1", converted_value ) );

	std::string converted_entry;
	ASSERT_TRUE( CEnumConverter::Convert( "EReflectionBitfieldTest", 3, converted_entry ) );
	ASSERT_TRUE( converted_entry == "BIT1 | BIT2" );

	ASSERT_TRUE( CEnumConverter::Convert( "EReflectionBitfieldTest", 0, converted_entry ) );
	ASSERT_TRUE( converted_entry == "NONE" );

	ASSERT_TRUE( CEnumConverter::Convert( "EReflectionBitfieldTest", 7, converted_entry ) );
	ASSERT_TRUE( converted_entry == "BIT1 | BIT2 | BIT3" );

	ASSERT_FALSE( CEnumConverter::Convert( "EReflectionBitfieldTes", 3, converted_entry ) );
	ASSERT_FALSE( CEnumConverter::Convert( "EReflectionBitfieldTest", 16, converted_entry ) );
}
