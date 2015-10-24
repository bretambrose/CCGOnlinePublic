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

#include "IPPlatform/StringUtils.h"

using namespace IP::String;

TEST( StringUtilsTests, String_To_WideString )
{
	std::wstring target;

	String_To_WideString( "", target );
	ASSERT_TRUE( target == std::wstring( L"" ) );

	String_To_WideString( "A", target );
	ASSERT_TRUE( target == std::wstring( L"A" ) );

	String_To_WideString( "AB", target );
	ASSERT_TRUE( target == std::wstring( L"AB" ) );

	String_To_WideString( "ABC", target );
	ASSERT_TRUE( target == std::wstring( L"ABC" ) );

	String_To_WideString( "ABCD", target );
	ASSERT_TRUE( target == std::wstring( L"ABCD" ) );

	String_To_WideString( "ABCDE", target );
	ASSERT_TRUE( target == std::wstring( L"ABCDE" ) );

	String_To_WideString( "ABCDEF", target );
	ASSERT_TRUE( target == std::wstring( L"ABCDEF" ) );

	String_To_WideString( "ABCDEFG", target );
	ASSERT_TRUE( target == std::wstring( L"ABCDEFG" ) );

	String_To_WideString( "Testing a string", target );
	ASSERT_TRUE( target == std::wstring( L"Testing a string" ) );

	for ( uint32_t i = 10; i < 200; ++i )
	{
		std::string input( i, 'A' );
		String_To_WideString( input, target );
		ASSERT_TRUE( target == std::wstring( i, 'A' ) );
	}
}

TEST( StringUtilsTests, WideString_To_String )
{
	std::string target;

	WideString_To_String( L"", target );
	ASSERT_TRUE( target == std::string( "" ) );

	WideString_To_String( L"A", target );
	ASSERT_TRUE( target == std::string( "A" ) );

	WideString_To_String( L"AB", target );
	ASSERT_TRUE( target == std::string( "AB" ) );

	WideString_To_String( L"ABC", target );
	ASSERT_TRUE( target == std::string( "ABC" ) );

	WideString_To_String( L"ABCD", target );
	ASSERT_TRUE( target == std::string( "ABCD" ) );

	WideString_To_String( L"ABCDE", target );
	ASSERT_TRUE( target == std::string( "ABCDE" ) );

	WideString_To_String( L"ABCDEF", target );
	ASSERT_TRUE( target == std::string( "ABCDEF" ) );

	WideString_To_String( L"ABCDEFG", target );
	ASSERT_TRUE( target == std::string( "ABCDEFG" ) );

	WideString_To_String( L"Testing a string", target );
	ASSERT_TRUE( target == std::string( "Testing a string" ) );

	for ( uint32_t i = 10; i < 200; ++i )
	{
		std::wstring input( i, 'A' );
		WideString_To_String( input, target );
		ASSERT_TRUE( target == std::string( i, 'A' ) );
	}
}

TEST( StringUtilsTests, String_To_Upper_Case )
{
	std::string target;

	To_Upper_Case( std::string( "" ), target );
	ASSERT_TRUE( target == std::string( "" ) );

	To_Upper_Case( std::string( "A" ), target );
	ASSERT_TRUE( target == std::string( "A" ) );

	To_Upper_Case( std::string( "a" ), target );
	ASSERT_TRUE( target == std::string( "A" ) );

	To_Upper_Case( std::string( "alphabet" ), target );
	ASSERT_TRUE( target == std::string( "ALPHABET" ) );

}

TEST( StringUtilsTests, WString_To_Upper_Case )
{
	std::wstring target;

	To_Upper_Case( std::wstring( L"" ), target );
	ASSERT_TRUE( target == std::wstring( L"" ) );

	To_Upper_Case( std::wstring( L"A" ), target );
	ASSERT_TRUE( target == std::wstring( L"A" ) );

	To_Upper_Case( std::wstring( L"a" ), target );
	ASSERT_TRUE( target == std::wstring( L"A" ) );

	To_Upper_Case( std::wstring( L"alphabet" ), target );
	ASSERT_TRUE( target == std::wstring( L"ALPHABET" ) );

}
