/**********************************************************************************************************************

	ThreadKeyTests.cpp
		defines unit tests for thread key related functionality

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

#include "Concurrency/ThreadKey.h"
#include "Concurrency/VirtualProcessConstants.h"

TEST( ThreadKeyTests, Constructors )
{
	SThreadKey part_key( TS_DATABASE, 5, 10 );
	EXPECT_TRUE( part_key.Get_Thread_Subject() == TS_DATABASE );
	EXPECT_TRUE( part_key.Get_Major_Sub_Key() == 5 );
	EXPECT_TRUE( part_key.Get_Minor_Sub_Key() == 10 );

	SThreadKey copied_key( part_key );
	EXPECT_TRUE( part_key.Get_Thread_Subject() == TS_DATABASE );
	EXPECT_TRUE( part_key.Get_Major_Sub_Key() == 5 );
	EXPECT_TRUE( part_key.Get_Minor_Sub_Key() == 10 );

	SThreadKey value_key( part_key.Get_Key() );
	EXPECT_TRUE( part_key.Get_Thread_Subject() == TS_DATABASE );
	EXPECT_TRUE( part_key.Get_Major_Sub_Key() == 5 );
	EXPECT_TRUE( part_key.Get_Minor_Sub_Key() == 10 );
}

TEST( ThreadKeyTests, Get_Set_Parts )
{
	SThreadKey mutated_key( TS_AI, 1, 2 );
	mutated_key.Set_Major_Sub_Key( 5 );
	EXPECT_TRUE( mutated_key.Get_Major_Sub_Key() == 5 );
	EXPECT_TRUE( mutated_key.Get_Thread_Subject() == TS_AI );
	EXPECT_TRUE( mutated_key.Get_Minor_Sub_Key() == 2 );

	mutated_key.Set_Minor_Sub_Key( 10 );
	EXPECT_TRUE( mutated_key.Get_Minor_Sub_Key() == 10 );
	EXPECT_TRUE( mutated_key.Get_Major_Sub_Key() == 5 );
	EXPECT_TRUE( mutated_key.Get_Thread_Subject() == TS_AI );

	EThreadSubject subject = TS_INVALID;
	uint16 major_sub_key = 0;
	uint16 minor_sub_key = 0;

	mutated_key.Get_Key_Parts( subject, major_sub_key, minor_sub_key );
	EXPECT_TRUE( subject == TS_AI );
	EXPECT_TRUE( major_sub_key == 5 );
	EXPECT_TRUE( minor_sub_key == 10 );
}

TEST( ThreadKeyTests, Is_Valid )
{
	SThreadKey bad_key1( 0 );
	EXPECT_FALSE( bad_key1.Is_Valid() );

	SThreadKey bad_key2( TS_INVALID, 1, 2 );
	EXPECT_FALSE( bad_key2.Is_Valid() );

	SThreadKey bad_key3( TS_UI, INVALID_SUB_KEY, 2 );
	EXPECT_FALSE( bad_key3.Is_Valid() );

	SThreadKey bad_key4( TS_NETWORK_CONNECTION_SET, 1, INVALID_SUB_KEY );
	EXPECT_FALSE( bad_key4.Is_Valid() );

	SThreadKey good_key1( TS_DATABASE, 1, 2 );
	EXPECT_TRUE( good_key1.Is_Valid() );

	SThreadKey good_key2( TS_AI, MAJOR_KEY_ALL, 2 );
	EXPECT_FALSE( good_key2.Is_Valid() );

	SThreadKey good_key3( TS_AI, 1, MINOR_KEY_ALL );
	EXPECT_FALSE( good_key3.Is_Valid() );
}

TEST( ThreadKeyTests, Needs_Sub_Key_Allocation )
{
	SThreadKey doesnt_need_key1( TS_DATABASE, 1, 2 );
	EXPECT_FALSE( doesnt_need_key1.Needs_Sub_Key_Allocation() );

	SThreadKey needs_key1( TS_AI, INVALID_SUB_KEY, 2 );
	EXPECT_TRUE( needs_key1.Needs_Sub_Key_Allocation() );

	SThreadKey needs_key2( TS_AI, 1, INVALID_SUB_KEY );
	EXPECT_TRUE( needs_key2.Needs_Sub_Key_Allocation() );
}

TEST( ThreadKeyTests, Matches )
{
	SThreadKey subject_star( TS_ALL, 1, 3 );
	EXPECT_TRUE( subject_star.Matches( SThreadKey( TS_DATABASE, 1, 3 ) ) );
	EXPECT_FALSE( subject_star.Matches( SThreadKey( TS_DATABASE, 2, 3 ) ) );
	EXPECT_FALSE( subject_star.Matches( SThreadKey( TS_DATABASE, 1, 4 ) ) );

	SThreadKey major_star( TS_LOGIC, MAJOR_KEY_ALL, 3 );
	EXPECT_TRUE( major_star.Matches( SThreadKey( TS_LOGIC, 1, 3 ) ) );
	EXPECT_FALSE( major_star.Matches( SThreadKey( TS_UI, 2, 3 ) ) );
	EXPECT_FALSE( major_star.Matches( SThreadKey( TS_LOGIC, 1, 4 ) ) );

	SThreadKey minor_star( TS_LOGGING, 2, MINOR_KEY_ALL );
	EXPECT_TRUE( minor_star.Matches( SThreadKey( TS_LOGGING, 2, 3 ) ) );
	EXPECT_FALSE( minor_star.Matches( SThreadKey( TS_UI, 2, 3 ) ) );
	EXPECT_FALSE( minor_star.Matches( SThreadKey( TS_LOGGING, 1, 4 ) ) );
}

TEST( ThreadKeyTests, Is_Unique )
{
	SThreadKey unique_key( TS_LOGGING, 4, 5 );
	EXPECT_TRUE( unique_key.Is_Unique() );

	SThreadKey star_key1( TS_ALL, 4, 5 );
	EXPECT_FALSE( star_key1.Is_Unique() );

	SThreadKey star_key2( TS_LOGGING, MAJOR_KEY_ALL, 5 );
	EXPECT_FALSE( star_key2.Is_Unique() );

	SThreadKey star_key3( TS_LOGGING, 4, MINOR_KEY_ALL );
	EXPECT_FALSE( star_key3.Is_Unique() );
}

TEST( ThreadKeyTests, Equality_Comparisons )
{
	SThreadKey key1( TS_AI, 1, 3 );
	SThreadKey key2( TS_DATABASE, 1, 3 );
	SThreadKey key3( TS_AI, 2, 3 );
	SThreadKey key4( TS_AI, 1, 4 );
	SThreadKey key5( TS_AI, 1, 3 );

	EXPECT_FALSE( key1 == key2 );
	EXPECT_FALSE( key1 == key3 );
	EXPECT_FALSE( key1 == key4 );
	EXPECT_TRUE( key1 == key5 );
}

TEST( ThreadKeyTests, STL_Container_Helper )
{
	SThreadKey key1( TS_AI, 1, 3 );
	SThreadKey key2( TS_DATABASE, 1, 3 );
	SThreadKey key3( TS_AI, 2, 3 );
	SThreadKey key4( TS_AI, 1, 4 );

	SThreadKeyContainerHelper helper;

	EXPECT_FALSE( helper( key1, key1 ) );
	EXPECT_TRUE( helper( key1, key2 ) );
	EXPECT_FALSE( helper( key2, key1 ) );
	EXPECT_TRUE( helper( key1, key3 ) );
	EXPECT_FALSE( helper( key3, key1 ) );
	EXPECT_TRUE( helper( key1, key4 ) );
	EXPECT_FALSE( helper( key4, key1 ) );
}