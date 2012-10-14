/**********************************************************************************************************************

	ProcessPropertiesTests.cpp
		defines unit tests for process properties related functionality

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

#include "Concurrency/ProcessProperties.h"
#include "Concurrency/ProcessConstants.h"
#include "Concurrency/ProcessSubject.h"

namespace EProcessSubjectExtended
{
	enum Enum
	{
		LOGIC = EProcessSubject::LOGGING + 1,
		NETWORK_CONNECTION_MANAGER,
		NETWORK_CONNECTION_SET,
		AI,
		UI,
		DATABASE

	};
}

TEST( ProcessPropertiesTests, Constructors )
{
	SProcessProperties props( EProcessSubjectExtended::DATABASE, 5, 10, 2 );
	EXPECT_TRUE( props.Get_Subject_As< EProcessSubjectExtended::Enum >() == EProcessSubjectExtended::DATABASE );
	EXPECT_TRUE( props.Get_Major_Part() == 5 );
	EXPECT_TRUE( props.Get_Minor_Part() == 10 );
	EXPECT_TRUE( props.Get_Mode_Part() == 2 );

	SProcessProperties copied_props( props );
	EXPECT_TRUE( copied_props.Get_Subject_As< EProcessSubjectExtended::Enum >() == EProcessSubjectExtended::DATABASE );
	EXPECT_TRUE( copied_props.Get_Major_Part() == 5 );
	EXPECT_TRUE( copied_props.Get_Minor_Part() == 10 );
	EXPECT_TRUE( copied_props.Get_Mode_Part() == 2 );

	SProcessProperties incomplete_props( EProcessSubjectExtended::LOGIC );
	EXPECT_TRUE( incomplete_props.Get_Subject_As< EProcessSubjectExtended::Enum >() == EProcessSubjectExtended::LOGIC );
	EXPECT_TRUE( incomplete_props.Get_Major_Part() == 1 );
	EXPECT_TRUE( incomplete_props.Get_Minor_Part() == 1 );
	EXPECT_TRUE( incomplete_props.Get_Mode_Part() == 1 );

	SProcessProperties incomplete_props2( EProcessSubjectExtended::LOGIC, 3 );
	EXPECT_TRUE( incomplete_props2.Get_Subject_As< EProcessSubjectExtended::Enum >() == EProcessSubjectExtended::LOGIC );
	EXPECT_TRUE( incomplete_props2.Get_Major_Part() == 3 );
	EXPECT_TRUE( incomplete_props2.Get_Minor_Part() == 1 );
	EXPECT_TRUE( incomplete_props2.Get_Mode_Part() == 1 );

	SProcessProperties incomplete_props3( EProcessSubjectExtended::LOGIC, 3, 2 );
	EXPECT_TRUE( incomplete_props3.Get_Subject_As< EProcessSubjectExtended::Enum >() == EProcessSubjectExtended::LOGIC );
	EXPECT_TRUE( incomplete_props3.Get_Major_Part() == 3 );
	EXPECT_TRUE( incomplete_props3.Get_Minor_Part() == 2 );
	EXPECT_TRUE( incomplete_props3.Get_Mode_Part() == 1 );
}

TEST( ProcessPropertiesTests, Is_Valid )
{
	SProcessProperties bad_props( EProcessSubjectExtended::LOGIC, 0, 0, 0 );
	EXPECT_FALSE( bad_props.Is_Valid() );

	SProcessProperties good_props( EProcessSubjectExtended::DATABASE );
	EXPECT_TRUE( good_props.Is_Valid() );

	SProcessProperties bad_props2( EProcessSubjectExtended::LOGIC, 1, 0, 0 );
	EXPECT_FALSE( bad_props2.Is_Valid() );

	SProcessProperties good_props2( EProcessSubjectExtended::DATABASE, 1 );
	EXPECT_TRUE( good_props2.Is_Valid() );

	SProcessProperties bad_props3( EProcessSubjectExtended::LOGIC, 1, 1, 0 );
	EXPECT_FALSE( bad_props3.Is_Valid() );

	SProcessProperties good_props3( EProcessSubjectExtended::DATABASE, 1, 1 );
	EXPECT_TRUE( good_props3.Is_Valid() );

	SProcessProperties good_props4( EProcessSubjectExtended::DATABASE, 1, 1, 1 );
	EXPECT_TRUE( good_props4.Is_Valid() );
}

TEST( ProcessPropertiesTests, Matches )
{
	SProcessProperties match_all;

	EXPECT_TRUE( match_all.Matches( SProcessProperties( EProcessSubjectExtended::LOGIC ) ) );
	EXPECT_TRUE( match_all.Matches( SProcessProperties( EProcessSubjectExtended::LOGIC, 2 ) ) );
	EXPECT_TRUE( match_all.Matches( SProcessProperties( EProcessSubjectExtended::LOGIC, 2, 2 ) ) );
	EXPECT_TRUE( match_all.Matches( SProcessProperties( EProcessSubjectExtended::LOGIC, 2, 2, 2 ) ) );

	SProcessProperties match_ai( EProcessSubjectExtended::AI, 0, 0, 0 );
	EXPECT_FALSE( match_ai.Matches( SProcessProperties( EProcessSubjectExtended::LOGIC ) ) );
	EXPECT_TRUE( match_ai.Matches( SProcessProperties( EProcessSubjectExtended::AI ) ) );
	EXPECT_TRUE( match_ai.Matches( SProcessProperties( EProcessSubjectExtended::AI, 1 ) ) );
	EXPECT_TRUE( match_ai.Matches( SProcessProperties( EProcessSubjectExtended::AI, 1, 1 ) ) );
	EXPECT_TRUE( match_ai.Matches( SProcessProperties( EProcessSubjectExtended::AI, 1, 1, 1 ) ) );

	SProcessProperties match_ai_two( EProcessSubjectExtended::AI, 2, 0, 0 );
	EXPECT_FALSE( match_ai_two.Matches( SProcessProperties( EProcessSubjectExtended::LOGIC ) ) );
	EXPECT_FALSE( match_ai_two.Matches( SProcessProperties( EProcessSubjectExtended::AI ) ) );
	EXPECT_FALSE( match_ai_two.Matches( SProcessProperties( EProcessSubjectExtended::AI, 3 ) ) );
	EXPECT_TRUE( match_ai_two.Matches( SProcessProperties( EProcessSubjectExtended::AI, 2 ) ) );
	EXPECT_TRUE( match_ai_two.Matches( SProcessProperties( EProcessSubjectExtended::AI, 2, 1 ) ) );
	EXPECT_TRUE( match_ai_two.Matches( SProcessProperties( EProcessSubjectExtended::AI, 2, 1, 1 ) ) );

	SProcessProperties match_ai_two_three( EProcessSubjectExtended::AI, 2, 3, 0 );
	EXPECT_FALSE( match_ai_two_three.Matches( SProcessProperties( EProcessSubjectExtended::LOGIC ) ) );
	EXPECT_FALSE( match_ai_two_three.Matches( SProcessProperties( EProcessSubjectExtended::AI ) ) );
	EXPECT_FALSE( match_ai_two_three.Matches( SProcessProperties( EProcessSubjectExtended::AI, 3 ) ) );
	EXPECT_FALSE( match_ai_two_three.Matches( SProcessProperties( EProcessSubjectExtended::AI, 2 ) ) );
	EXPECT_FALSE( match_ai_two_three.Matches( SProcessProperties( EProcessSubjectExtended::AI, 2, 1 ) ) );
	EXPECT_TRUE( match_ai_two_three.Matches( SProcessProperties( EProcessSubjectExtended::AI, 2, 3 ) ) );
	EXPECT_TRUE( match_ai_two_three.Matches( SProcessProperties( EProcessSubjectExtended::AI, 2, 3, 2 ) ) );

	SProcessProperties match_ai_two_three_four( EProcessSubjectExtended::AI, 2, 3, 4 );
	EXPECT_FALSE( match_ai_two_three_four.Matches( SProcessProperties( EProcessSubjectExtended::LOGIC ) ) );
	EXPECT_FALSE( match_ai_two_three_four.Matches( SProcessProperties( EProcessSubjectExtended::AI ) ) );
	EXPECT_FALSE( match_ai_two_three_four.Matches( SProcessProperties( EProcessSubjectExtended::AI, 3 ) ) );
	EXPECT_FALSE( match_ai_two_three_four.Matches( SProcessProperties( EProcessSubjectExtended::AI, 2 ) ) );
	EXPECT_FALSE( match_ai_two_three_four.Matches( SProcessProperties( EProcessSubjectExtended::AI, 2, 1 ) ) );
	EXPECT_FALSE( match_ai_two_three_four.Matches( SProcessProperties( EProcessSubjectExtended::AI, 2, 3 ) ) );
	EXPECT_FALSE( match_ai_two_three_four.Matches( SProcessProperties( EProcessSubjectExtended::AI, 2, 3, 2 ) ) );
	EXPECT_TRUE( match_ai_two_three_four.Matches( SProcessProperties( EProcessSubjectExtended::AI, 2, 3, 4 ) ) );
}

TEST( ProcessPropertiesTests, Equality_Comparisons )
{
	SProcessProperties props1( EProcessSubjectExtended::AI, 1, 3 );
	SProcessProperties props2( EProcessSubjectExtended::DATABASE, 1, 3 );
	SProcessProperties props3( EProcessSubjectExtended::AI, 2, 3 );
	SProcessProperties props4( EProcessSubjectExtended::AI, 1, 4 );
	SProcessProperties props5( EProcessSubjectExtended::AI, 1, 3 );
	SProcessProperties props6( EProcessSubjectExtended::AI, 1, 3, 0 );

	EXPECT_FALSE( props1 == props2 );
	EXPECT_FALSE( props1 == props3 );
	EXPECT_FALSE( props1 == props4 );
	EXPECT_FALSE( props1 == props6 );
	EXPECT_TRUE( props1 == props5 );
}

TEST( ProcessPropertiesTests, STL_Container_Helper )
{
	SProcessProperties key1( EProcessSubjectExtended::AI, 1, 3 );
	SProcessProperties key2( EProcessSubjectExtended::DATABASE, 1, 3 );
	SProcessProperties key3( EProcessSubjectExtended::AI, 2, 3 );
	SProcessProperties key4( EProcessSubjectExtended::AI, 1, 4 );

	SProcessPropertiesContainerHelper helper;

	EXPECT_FALSE( helper( key1, key1 ) );
	EXPECT_TRUE( helper( key1, key2 ) );
	EXPECT_FALSE( helper( key2, key1 ) );
	EXPECT_TRUE( helper( key1, key3 ) );
	EXPECT_FALSE( helper( key3, key1 ) );
	EXPECT_TRUE( helper( key1, key4 ) );
	EXPECT_FALSE( helper( key4, key1 ) );
}