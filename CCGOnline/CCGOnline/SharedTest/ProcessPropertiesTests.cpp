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

#include "Concurrency/VirtualProcessProperties.h"
#include "Concurrency/VirtualProcessConstants.h"
#include "Concurrency/VirtualProcessSubject.h"

namespace EVirtualProcessSubjectExtended
{
	enum Enum
	{
		LOGIC = EVirtualProcessSubject::LOGGING + 1,
		NETWORK_CONNECTION_MANAGER,
		NETWORK_CONNECTION_SET,
		AI,
		UI,
		DATABASE

	};
}

TEST( ProcessPropertiesTests, Constructors )
{
	SProcessProperties props( EVirtualProcessSubjectExtended::DATABASE, 5, 10, 2 );
	EXPECT_TRUE( props.Get_Subject_As< EVirtualProcessSubjectExtended::Enum >() == EVirtualProcessSubjectExtended::DATABASE );
	EXPECT_TRUE( props.Get_Major_Part() == 5 );
	EXPECT_TRUE( props.Get_Minor_Part() == 10 );
	EXPECT_TRUE( props.Get_Mode_Part() == 2 );

	SProcessProperties copied_props( props );
	EXPECT_TRUE( copied_props.Get_Subject_As< EVirtualProcessSubjectExtended::Enum >() == EVirtualProcessSubjectExtended::DATABASE );
	EXPECT_TRUE( copied_props.Get_Major_Part() == 5 );
	EXPECT_TRUE( copied_props.Get_Minor_Part() == 10 );
	EXPECT_TRUE( copied_props.Get_Mode_Part() == 2 );

	SProcessProperties incomplete_props( EVirtualProcessSubjectExtended::LOGIC );
	EXPECT_TRUE( incomplete_props.Get_Subject_As< EVirtualProcessSubjectExtended::Enum >() == EVirtualProcessSubjectExtended::LOGIC );
	EXPECT_TRUE( incomplete_props.Get_Major_Part() == 1 );
	EXPECT_TRUE( incomplete_props.Get_Minor_Part() == 1 );
	EXPECT_TRUE( incomplete_props.Get_Mode_Part() == 1 );

	SProcessProperties incomplete_props2( EVirtualProcessSubjectExtended::LOGIC, 3 );
	EXPECT_TRUE( incomplete_props2.Get_Subject_As< EVirtualProcessSubjectExtended::Enum >() == EVirtualProcessSubjectExtended::LOGIC );
	EXPECT_TRUE( incomplete_props2.Get_Major_Part() == 3 );
	EXPECT_TRUE( incomplete_props2.Get_Minor_Part() == 1 );
	EXPECT_TRUE( incomplete_props2.Get_Mode_Part() == 1 );

	SProcessProperties incomplete_props3( EVirtualProcessSubjectExtended::LOGIC, 3, 2 );
	EXPECT_TRUE( incomplete_props3.Get_Subject_As< EVirtualProcessSubjectExtended::Enum >() == EVirtualProcessSubjectExtended::LOGIC );
	EXPECT_TRUE( incomplete_props3.Get_Major_Part() == 3 );
	EXPECT_TRUE( incomplete_props3.Get_Minor_Part() == 2 );
	EXPECT_TRUE( incomplete_props3.Get_Mode_Part() == 1 );
}

TEST( ProcessPropertiesTests, Is_Valid )
{
	SProcessProperties bad_props( EVirtualProcessSubjectExtended::LOGIC, 0, 0, 0 );
	EXPECT_FALSE( bad_props.Is_Valid() );

	SProcessProperties good_props( EVirtualProcessSubjectExtended::DATABASE );
	EXPECT_TRUE( good_props.Is_Valid() );

	SProcessProperties bad_props2( EVirtualProcessSubjectExtended::LOGIC, 1, 0, 0 );
	EXPECT_FALSE( bad_props2.Is_Valid() );

	SProcessProperties good_props2( EVirtualProcessSubjectExtended::DATABASE, 1 );
	EXPECT_TRUE( good_props2.Is_Valid() );

	SProcessProperties bad_props3( EVirtualProcessSubjectExtended::LOGIC, 1, 1, 0 );
	EXPECT_FALSE( bad_props3.Is_Valid() );

	SProcessProperties good_props3( EVirtualProcessSubjectExtended::DATABASE, 1, 1 );
	EXPECT_TRUE( good_props3.Is_Valid() );

	SProcessProperties good_props4( EVirtualProcessSubjectExtended::DATABASE, 1, 1, 1 );
	EXPECT_TRUE( good_props4.Is_Valid() );
}

TEST( ProcessPropertiesTests, Matches )
{
	SProcessProperties match_all;

	EXPECT_TRUE( match_all.Matches( SProcessProperties( EVirtualProcessSubjectExtended::LOGIC ) ) );
	EXPECT_TRUE( match_all.Matches( SProcessProperties( EVirtualProcessSubjectExtended::LOGIC, 2 ) ) );
	EXPECT_TRUE( match_all.Matches( SProcessProperties( EVirtualProcessSubjectExtended::LOGIC, 2, 2 ) ) );
	EXPECT_TRUE( match_all.Matches( SProcessProperties( EVirtualProcessSubjectExtended::LOGIC, 2, 2, 2 ) ) );

	SProcessProperties match_ai( EVirtualProcessSubjectExtended::AI, 0, 0, 0 );
	EXPECT_FALSE( match_ai.Matches( SProcessProperties( EVirtualProcessSubjectExtended::LOGIC ) ) );
	EXPECT_TRUE( match_ai.Matches( SProcessProperties( EVirtualProcessSubjectExtended::AI ) ) );
	EXPECT_TRUE( match_ai.Matches( SProcessProperties( EVirtualProcessSubjectExtended::AI, 1 ) ) );
	EXPECT_TRUE( match_ai.Matches( SProcessProperties( EVirtualProcessSubjectExtended::AI, 1, 1 ) ) );
	EXPECT_TRUE( match_ai.Matches( SProcessProperties( EVirtualProcessSubjectExtended::AI, 1, 1, 1 ) ) );

	SProcessProperties match_ai_two( EVirtualProcessSubjectExtended::AI, 2, 0, 0 );
	EXPECT_FALSE( match_ai_two.Matches( SProcessProperties( EVirtualProcessSubjectExtended::LOGIC ) ) );
	EXPECT_FALSE( match_ai_two.Matches( SProcessProperties( EVirtualProcessSubjectExtended::AI ) ) );
	EXPECT_FALSE( match_ai_two.Matches( SProcessProperties( EVirtualProcessSubjectExtended::AI, 3 ) ) );
	EXPECT_TRUE( match_ai_two.Matches( SProcessProperties( EVirtualProcessSubjectExtended::AI, 2 ) ) );
	EXPECT_TRUE( match_ai_two.Matches( SProcessProperties( EVirtualProcessSubjectExtended::AI, 2, 1 ) ) );
	EXPECT_TRUE( match_ai_two.Matches( SProcessProperties( EVirtualProcessSubjectExtended::AI, 2, 1, 1 ) ) );

	SProcessProperties match_ai_two_three( EVirtualProcessSubjectExtended::AI, 2, 3, 0 );
	EXPECT_FALSE( match_ai_two_three.Matches( SProcessProperties( EVirtualProcessSubjectExtended::LOGIC ) ) );
	EXPECT_FALSE( match_ai_two_three.Matches( SProcessProperties( EVirtualProcessSubjectExtended::AI ) ) );
	EXPECT_FALSE( match_ai_two_three.Matches( SProcessProperties( EVirtualProcessSubjectExtended::AI, 3 ) ) );
	EXPECT_FALSE( match_ai_two_three.Matches( SProcessProperties( EVirtualProcessSubjectExtended::AI, 2 ) ) );
	EXPECT_FALSE( match_ai_two_three.Matches( SProcessProperties( EVirtualProcessSubjectExtended::AI, 2, 1 ) ) );
	EXPECT_TRUE( match_ai_two_three.Matches( SProcessProperties( EVirtualProcessSubjectExtended::AI, 2, 3 ) ) );
	EXPECT_TRUE( match_ai_two_three.Matches( SProcessProperties( EVirtualProcessSubjectExtended::AI, 2, 3, 2 ) ) );

	SProcessProperties match_ai_two_three_four( EVirtualProcessSubjectExtended::AI, 2, 3, 4 );
	EXPECT_FALSE( match_ai_two_three_four.Matches( SProcessProperties( EVirtualProcessSubjectExtended::LOGIC ) ) );
	EXPECT_FALSE( match_ai_two_three_four.Matches( SProcessProperties( EVirtualProcessSubjectExtended::AI ) ) );
	EXPECT_FALSE( match_ai_two_three_four.Matches( SProcessProperties( EVirtualProcessSubjectExtended::AI, 3 ) ) );
	EXPECT_FALSE( match_ai_two_three_four.Matches( SProcessProperties( EVirtualProcessSubjectExtended::AI, 2 ) ) );
	EXPECT_FALSE( match_ai_two_three_four.Matches( SProcessProperties( EVirtualProcessSubjectExtended::AI, 2, 1 ) ) );
	EXPECT_FALSE( match_ai_two_three_four.Matches( SProcessProperties( EVirtualProcessSubjectExtended::AI, 2, 3 ) ) );
	EXPECT_FALSE( match_ai_two_three_four.Matches( SProcessProperties( EVirtualProcessSubjectExtended::AI, 2, 3, 2 ) ) );
	EXPECT_TRUE( match_ai_two_three_four.Matches( SProcessProperties( EVirtualProcessSubjectExtended::AI, 2, 3, 4 ) ) );
}

TEST( ProcessPropertiesTests, Equality_Comparisons )
{
	SProcessProperties props1( EVirtualProcessSubjectExtended::AI, 1, 3 );
	SProcessProperties props2( EVirtualProcessSubjectExtended::DATABASE, 1, 3 );
	SProcessProperties props3( EVirtualProcessSubjectExtended::AI, 2, 3 );
	SProcessProperties props4( EVirtualProcessSubjectExtended::AI, 1, 4 );
	SProcessProperties props5( EVirtualProcessSubjectExtended::AI, 1, 3 );
	SProcessProperties props6( EVirtualProcessSubjectExtended::AI, 1, 3, 0 );

	EXPECT_FALSE( props1 == props2 );
	EXPECT_FALSE( props1 == props3 );
	EXPECT_FALSE( props1 == props4 );
	EXPECT_FALSE( props1 == props6 );
	EXPECT_TRUE( props1 == props5 );
}

TEST( ProcessPropertiesTests, STL_Container_Helper )
{
	SProcessProperties key1( EVirtualProcessSubjectExtended::AI, 1, 3 );
	SProcessProperties key2( EVirtualProcessSubjectExtended::DATABASE, 1, 3 );
	SProcessProperties key3( EVirtualProcessSubjectExtended::AI, 2, 3 );
	SProcessProperties key4( EVirtualProcessSubjectExtended::AI, 1, 4 );

	SProcessPropertiesContainerHelper helper;

	EXPECT_FALSE( helper( key1, key1 ) );
	EXPECT_TRUE( helper( key1, key2 ) );
	EXPECT_FALSE( helper( key2, key1 ) );
	EXPECT_TRUE( helper( key1, key3 ) );
	EXPECT_FALSE( helper( key3, key1 ) );
	EXPECT_TRUE( helper( key1, key4 ) );
	EXPECT_FALSE( helper( key4, key1 ) );
}