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

#include <gtest/gtest.h>

#include <IPCore/Utils/StringUtils.h>

using namespace IP::StringUtils;

TEST( StringUtilsTests, String_To_WideString )
{
	IP::WString target;

	String_To_WideString( "", target );
	ASSERT_TRUE( target == IP::WString( L"" ) );

	String_To_WideString( "A", target );
	ASSERT_TRUE( target == IP::WString( L"A" ) );

	String_To_WideString( "AB", target );
	ASSERT_TRUE( target == IP::WString( L"AB" ) );

	String_To_WideString( "ABC", target );
	ASSERT_TRUE( target == IP::WString( L"ABC" ) );

	String_To_WideString( "ABCD", target );
	ASSERT_TRUE( target == IP::WString( L"ABCD" ) );

	String_To_WideString( "ABCDE", target );
	ASSERT_TRUE( target == IP::WString( L"ABCDE" ) );

	String_To_WideString( "ABCDEF", target );
	ASSERT_TRUE( target == IP::WString( L"ABCDEF" ) );

	String_To_WideString( "ABCDEFG", target );
	ASSERT_TRUE( target == IP::WString( L"ABCDEFG" ) );

	String_To_WideString( "Testing a string", target );
	ASSERT_TRUE( target == IP::WString( L"Testing a string" ) );
}

TEST( StringUtilsTests, WideString_To_String )
{
	IP::String target;

	WideString_To_String( L"", target );
	ASSERT_TRUE( target == IP::String( "" ) );

	WideString_To_String( L"A", target );
	ASSERT_TRUE( target == IP::String( "A" ) );

	WideString_To_String( L"AB", target );
	ASSERT_TRUE( target == IP::String( "AB" ) );

	WideString_To_String( L"ABC", target );
	ASSERT_TRUE( target == IP::String( "ABC" ) );

	WideString_To_String( L"ABCD", target );
	ASSERT_TRUE( target == IP::String( "ABCD" ) );

	WideString_To_String( L"ABCDE", target );
	ASSERT_TRUE( target == IP::String( "ABCDE" ) );

	WideString_To_String( L"ABCDEF", target );
	ASSERT_TRUE( target == IP::String( "ABCDEF" ) );

	WideString_To_String( L"ABCDEFG", target );
	ASSERT_TRUE( target == IP::String( "ABCDEFG" ) );

	WideString_To_String( L"Testing a string", target );
	ASSERT_TRUE( target == IP::String( "Testing a string" ) );
}

TEST( StringUtilsTests, String_To_Upper_Case )
{
	IP::String target;

	To_Upper_Case( IP::String( "" ), target );
	ASSERT_TRUE( target == IP::String( "" ) );

	To_Upper_Case( IP::String( "A" ), target );
	ASSERT_TRUE( target == IP::String( "A" ) );

	To_Upper_Case( IP::String( "a" ), target );
	ASSERT_TRUE( target == IP::String( "A" ) );

	To_Upper_Case( IP::String( "alphabet" ), target );
	ASSERT_TRUE( target == IP::String( "ALPHABET" ) );

}

TEST( StringUtilsTests, WString_To_Upper_Case )
{
	IP::WString target;

	To_Upper_Case( IP::WString( L"" ), target );
	ASSERT_TRUE( target == IP::WString( L"" ) );

	To_Upper_Case( IP::WString( L"A" ), target );
	ASSERT_TRUE( target == IP::WString( L"A" ) );

	To_Upper_Case( IP::WString( L"a" ), target );
	ASSERT_TRUE( target == IP::WString( L"A" ) );

	To_Upper_Case( IP::WString( L"alphabet" ), target );
	ASSERT_TRUE( target == IP::WString( L"ALPHABET" ) );

}
