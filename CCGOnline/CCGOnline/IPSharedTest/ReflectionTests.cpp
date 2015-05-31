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

#include "ReflectionTests.h"
#include "IPShared/EnumConversion.h"

TEST( ReflectionTests, Normal )
{
	EReflectionTest converted_value = RT_INVALID;
	ASSERT_TRUE( CEnumConverter::Convert( "Entry1", converted_value ) );
	ASSERT_TRUE( converted_value == RT_ENTRY1 );

	ASSERT_TRUE( CEnumConverter::Convert( "eNTRy2", converted_value ) );
	ASSERT_TRUE( converted_value == RT_ENTRY2 );

	ASSERT_TRUE( CEnumConverter::Convert( "eNTRy3", converted_value ) );
	ASSERT_TRUE( converted_value == RT_ENTRY3 );

	ASSERT_TRUE( CEnumConverter::Convert( "eNTRy4", converted_value ) );
	ASSERT_TRUE( converted_value == RT_ENTRY4 );

	ASSERT_FALSE( CEnumConverter::Convert( "Entryy1", converted_value ) );

	std::string converted_entry;
	ASSERT_TRUE( CEnumConverter::Convert( RT_ENTRY2, converted_entry ) );
	ASSERT_TRUE( converted_entry == "ENTRY2" );

	ASSERT_TRUE( CEnumConverter::Convert( RT_INVALID, converted_entry ) );
	ASSERT_TRUE( converted_entry == "INVALID" );

	ASSERT_FALSE( CEnumConverter::Convert( static_cast< EReflectionTest >( 10 ), converted_entry ) );
}

TEST( ReflectionTests, Bitfield )
{
	EReflectionBitfieldTest converted_value = RBT_NONE;
	ASSERT_TRUE( CEnumConverter::Convert( "NoNe", converted_value ) );
	ASSERT_TRUE( converted_value == RBT_NONE );

	ASSERT_TRUE( CEnumConverter::Convert( "Bit3", converted_value ) );
	ASSERT_TRUE( converted_value == 4 );

	ASSERT_TRUE( CEnumConverter::Convert( "BIt3|Bit1", converted_value ) );
	ASSERT_TRUE( converted_value == 5 );

	ASSERT_TRUE( CEnumConverter::Convert( "Bit2| BiT8", converted_value ) );
	ASSERT_TRUE( converted_value == 130 );

	ASSERT_TRUE( CEnumConverter::Convert( "BIT1| Bit2|Bit3 |Bit8", converted_value ) );
	ASSERT_TRUE( converted_value == 135 );

	ASSERT_FALSE( CEnumConverter::Convert( "Bitt1", converted_value ) );

	std::string converted_entry;
	ASSERT_TRUE( CEnumConverter::Convert( static_cast< EReflectionBitfieldTest >( 3 ), converted_entry ) );
	ASSERT_TRUE( converted_entry == "BIT1 | BIT2" );

	ASSERT_TRUE( CEnumConverter::Convert( static_cast< EReflectionBitfieldTest >( 0 ), converted_entry ) );
	ASSERT_TRUE( converted_entry == "NONE" );

	ASSERT_TRUE( CEnumConverter::Convert( static_cast< EReflectionBitfieldTest >( 7 ), converted_entry ) );
	ASSERT_TRUE( converted_entry == "BIT1 | BIT2 | BIT3" );

	ASSERT_FALSE( CEnumConverter::Convert( static_cast< EReflectionBitfieldTest >( 16 ), converted_entry ) );
}

TEST( ReflectionTests, Namespace )
{
	TestNameSpace::Test converted_value = TestNameSpace::NONE;
	ASSERT_TRUE( CEnumConverter::Convert( "Test2", converted_value ) );
	ASSERT_TRUE( converted_value == TestNameSpace::TEST2 );

	ASSERT_TRUE( CEnumConverter::Convert( "TEST3", converted_value ) );
	ASSERT_TRUE( converted_value == TestNameSpace::TEST3 );

	ASSERT_FALSE( CEnumConverter::Convert( "Entryy1", converted_value ) );

	std::string converted_entry;
	ASSERT_TRUE( CEnumConverter::Convert( TestNameSpace::TEST2, converted_entry ) );
	ASSERT_TRUE( converted_entry == "TEST2" );

	ASSERT_FALSE( CEnumConverter::Convert( static_cast< TestNameSpace::Test >( 10 ), converted_entry ) );
}

TEST( ReflectionTests, Inheritance1 )
{
	BaseTest converted_value = BT_NONE;
	ASSERT_TRUE( CEnumConverter::Convert( "Test2", converted_value ) );
	ASSERT_TRUE( converted_value == BT_TEST2 );

	ASSERT_TRUE( CEnumConverter::Convert( "TEST5", converted_value ) );
	ASSERT_TRUE( converted_value == static_cast< BaseTest >( 5 ) );

	ASSERT_FALSE( CEnumConverter::Convert( "Entryy1", converted_value ) );

	std::string converted_entry;
	ASSERT_TRUE( CEnumConverter::Convert( BT_TEST2, converted_entry ) );
	ASSERT_TRUE( converted_entry == "TEST2" );

	ASSERT_TRUE( CEnumConverter::Convert( static_cast< BaseTest >( DT_TEST6 ), converted_entry ) );
	ASSERT_TRUE( converted_entry == "TEST6" );

	ASSERT_FALSE( CEnumConverter::Convert( static_cast< BaseTest >( 10 ), converted_entry ) );
}

TEST( ReflectionTests, Inheritance2 )
{
	DerivedTest converted_value = static_cast< DerivedTest >( BT_NONE );
	ASSERT_TRUE( CEnumConverter::Convert( "Test2", converted_value ) );
	ASSERT_TRUE( converted_value == static_cast< DerivedTest >( BT_TEST2 ) );

	ASSERT_TRUE( CEnumConverter::Convert( "TEST5", converted_value ) );
	ASSERT_TRUE( converted_value == DT_TEST5 );

	ASSERT_FALSE( CEnumConverter::Convert( "Entryy1", converted_value ) );

	std::string converted_entry;
	ASSERT_TRUE( CEnumConverter::Convert( static_cast< DerivedTest >( BT_TEST2 ), converted_entry ) );
	ASSERT_TRUE( converted_entry == "TEST2" );

	ASSERT_TRUE( CEnumConverter::Convert( DT_TEST6, converted_entry ) );
	ASSERT_TRUE( converted_entry == "TEST6" );

	ASSERT_FALSE( CEnumConverter::Convert( static_cast< DerivedTest >( 10 ), converted_entry ) );
}