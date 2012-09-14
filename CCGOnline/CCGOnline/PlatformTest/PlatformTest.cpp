/**********************************************************************************************************************

	PlatformTest.cpp
		the entry point for the console application that runs the unit tests

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

#ifdef NEVER

#include <Windows.h>

enum MonsterFlags
{
	MS_HasMeleeAbilities = 0x01,
	MS_HasSpellAbilities = 0x02,
	MS_Invisible = 0x04,
	MS_Unkillable = 0x08,
};

/*
bool HasSpellAbilities( unsigned int monsterFlag )
{
	return ( monsterFlag & MS_HasSpellAbilities ) != 0; 
}
*/

unsigned int Some_Function( unsigned int monsterFlag, unsigned int monsterLevel )
{
	return monsterFlag + monsterLevel;
}


bool HasSpellAbilities( MonsterFlags monsterFlag )
{
	return ( monsterFlag & MS_HasSpellAbilities ) != 0; 
}


enum PlayerFlags
{
	PF_Blah
};

void Bad_Stuff( void )
{
	PlayerFlags player_flags = PF_Blah;
	HasSpellAbilities( player_flags );


}

unsigned int SetMonsterInvisible( unsigned int currentFlags, bool isInvisible )
{
	if ( isInvisible )
	{
		return currentFlags | MS_Invisible;
	}
	else
	{
		return currentFlags & ( ~MS_Invisible );
	}
}

bool Platform_Get_Cursor_Position( int &x, int &y )
{
	POINT cursor_location;
	if ( ::GetCursorPos( &cursor_location ) != 0 )
	{
		x = cursor_location.x;
		y = cursor_location.y;
		return true;
	}

	// check last error state here and convert to a platform-agnostic function-specific error code if appropriate
	return false;
}

typedef struct 
{
	float x, y, z;
} VECTOR;

static const float CLOSE_ENOUGH_TO_ZERO = 0.0000001f;

// the "correct" version that uses sqrt.  There are plenty of less precise but faster implementations out
// there that avoid sqrt
void Normalize_Vector( VECTOR *v )
{
	// you could pull the coordinates into locals here, but I assume the compiler is smart enough to figure that out
	// by itself
	float length = ::sqrt( v->x * v->x + v->y * v->y + v->z * v->z );
	if ( length < CLOSE_ENOUGH_TO_ZERO )	
	{
		return;
	}

	float one_over_length = 1.0f / length;

	v->x *= one_over_length;
	v->y *= one_over_length;
	v->z *= one_over_length;
}

enum ENewStrcpyResults
{
	NSR_SUCCESS,
	NSR_SUCCESS_WITH_CLIP,
	NSR_NULL_SOURCE,
	NSR_BAD_DEST
};

ENewStrcpyResults New_strcpy_s( const char *source_buffer, char *dest_buffer, unsigned int dest_buffer_size )
{
	if ( source_buffer == NULL )
	{
		return NSR_NULL_SOURCE;
	}

	if ( dest_buffer == NULL || dest_buffer_size == 0 )
	{
		return NSR_BAD_DEST;
	}

	for ( unsigned int i = 0; i < dest_buffer_size; ++i )
	{
		char source_char = source_buffer[ i ];

		dest_buffer[ i ] = source_char;
		if ( source_char == 0 )
		{
			return NSR_SUCCESS;
		}
	}

	dest_buffer[ dest_buffer_size - 1 ] = 0;
	return NSR_SUCCESS_WITH_CLIP;
}

TEST( StrCpyTests, NullSource )
{
	const char *null_source = NULL;
	char dest_buffer[ 10 ];

	ASSERT_TRUE( New_strcpy_s( null_source, dest_buffer, sizeof( dest_buffer ) / sizeof( char ) ) == NSR_NULL_SOURCE );
}

TEST( StrCpyTests, NullDest )
{
	const char *valid_source = "valid";
	char *bad_dest_buffer = NULL;

	ASSERT_TRUE( New_strcpy_s( valid_source, bad_dest_buffer, 10 ) == NSR_BAD_DEST );
}

TEST( StrCpyTests, ZeroLengthDest )
{
	const char *valid_source = "valid";
	char dest_buffer[ 10 ];

	ASSERT_TRUE( New_strcpy_s( valid_source, dest_buffer, 0 ) == NSR_BAD_DEST );
}

TEST( StrCpyTests, Success )
{
	const char *valid_source = "valid";
	char dest_buffer[ 10 ];

	ASSERT_TRUE( New_strcpy_s( valid_source, dest_buffer, sizeof( dest_buffer ) / sizeof( char ) ) == NSR_SUCCESS );
}

TEST( StrCpyTests, DestTooSmall )
{
	const char *long_source = "this is a long long string";
	char dest_buffer[ 10 ];

	ASSERT_TRUE( New_strcpy_s( long_source, dest_buffer, sizeof( dest_buffer ) / sizeof( char ) ) == NSR_SUCCESS_WITH_CLIP );
}

// changed signature, to return success/failure, with value as output param
// all "return 0;" where an error condition was hit have been replaced with
// "return false;"
bool ParseHexString( const char *hexString, unsigned int &value )	
{
	// added null check
	if ( hexString == NULL )
	{
		return false;
	}

	// switched the order of the if checks, otherwise you deref off the end if it's zero-length.
	if ( hexString[ 0 ] != '0' )
	{
		return false;
	}

	// added capital 'X' check to this statement
	if ( hexString[ 1 ] != 'x' && hexString[ 1 ] != 'X' )
	{
		return false;
	}

	// let's make sure that "0x" is treated as invalid
	if ( hexString[ 2 ] == 0 )
	{
		return false;
	}

	// value declaration was in the wrong place and it was not initialized to zero
	// also I'm more comfortable making value an unsigned int, although I don't think it matters here
	value = 0;

	// ouch, loop all kinds of bad: 
	// (1) p needs to be a const char *
	// (2) hexString[ 2 ] is a char, use pointer arithmetic instead
	// (3) the exit condition needs to dereference the pointer p
	// (4) &p++ won't increment p, it'll just return the value of the next address after &p while making no side effect
	for ( const char *p = hexString + 2; *p != 0; ++p )
	{
		// value now initialized outside of loop, now an output parameter
		char hexDigit = *p;	// i doesn't exist and p is our iteration variable
		if ( hexDigit >= '0' && hexDigit <= '9' ) // & becomes &&, < becomes <=
		{
			hexDigit -= '0';
		}
		else if ( hexDigit >= 'A' && hexDigit <= 'F' )	// < becomes <=, else added
		{
			hexDigit = hexDigit - 'A' + 10;	// needs to transform to 10..15, not 0..5
		}
		else if ( hexDigit >= 'a' && hexDigit <= 'f' )	// missing case, presumably want to support lowercase hex too
		{
			hexDigit = hexDigit - 'a' + 10;  // needs to transform to 10..15, not 0..5
		}
		else
		{
			return false;
		}

		value *= 16;	
		value += hexDigit;

		// we could add overflow checking by tracking the number of digits seen while value is non-zero.  As soon as we see
		// 9 "significant" digits, we have overflow.  By only counting when value is non-zero, we avoid counting
		// leading zeros as significiant, so 0x0000000000000001 is fine, but 0x100000000 becomes bad.
	}

	return true;
}

TEST( ParseHexTests, NullSource )
{
	unsigned int value = 0;
	ASSERT_FALSE( ParseHexString( NULL, value ) );
}

TEST( ParseHexTests, CrappyBeginnings )
{
	unsigned int value = 0;
	ASSERT_FALSE( ParseHexString( "", value ) );
	ASSERT_FALSE( ParseHexString( "0", value ) );
	ASSERT_FALSE( ParseHexString( "b", value ) );
	ASSERT_FALSE( ParseHexString( "0x", value ) );
	ASSERT_FALSE( ParseHexString( "0f", value ) );
}

TEST( ParseHexTests, BadHex )
{
	unsigned int value = 0;
	ASSERT_FALSE( ParseHexString( "0xFA1L", value ) );
}

TEST( ParseHexTests, Good )
{
	unsigned int value = 0;
	ASSERT_TRUE( ParseHexString( "0xFF", value ) );
	ASSERT_TRUE( value == 255 );

	ASSERT_TRUE( ParseHexString( "0xff", value ) );
	ASSERT_TRUE( value == 255 );

	ASSERT_TRUE( ParseHexString( "0xa1bc", value ) );
	ASSERT_TRUE( value == 0xa1bc );

	ASSERT_TRUE( ParseHexString( "0XA1bF", value ) );
	ASSERT_TRUE( value == 0XA1bF );

	ASSERT_TRUE( ParseHexString( "0Xdeadbeef", value ) );
	ASSERT_TRUE( value == 0Xdeadbeef );

	ASSERT_TRUE( ParseHexString( "0x0000000001", value ) );
	ASSERT_TRUE( value == 1 );
}

#endif // NEVER

int main(int argc, wchar_t* argv[])
{


	::testing::InitGoogleTest(&argc, argv);
	RUN_ALL_TESTS();

	return 0;
}


