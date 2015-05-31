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

#include "IPPlatform/EnumUtils.h"

enum class EUtilFlagTest
{
	None = 0,
	Flag1 = 1 << 0,
	Flag2 = 1 << 1,
	Flag3 = 1 << 2
};

TEST( EnumUtilsTests, Build_Mask )
{
	EUtilFlagTest empty_value = Make_Enum_Mask( EUtilFlagTest::None );
	ASSERT_TRUE( empty_value == EUtilFlagTest::None );

	EUtilFlagTest flag1_value = Make_Enum_Mask( EUtilFlagTest::Flag1 );
	ASSERT_TRUE( flag1_value == EUtilFlagTest::Flag1 );

	EUtilFlagTest mask_value = Make_Enum_Mask( EUtilFlagTest::Flag1, EUtilFlagTest::Flag2 );
	ASSERT_TRUE( static_cast< uint32_t >( mask_value ) == 3 );

	ASSERT_TRUE( Is_An_Enum_Flag_Set( mask_value, flag1_value ) );
	ASSERT_TRUE( Are_All_Enum_Flags_Set( mask_value, flag1_value ) );
	ASSERT_FALSE( Are_All_Enum_Flags_Set( flag1_value, mask_value ) );
}

