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

#pragma once
 
//:EnumBegin()
enum EReflectionTest
{
	RT_INVALID = 0,		//:EnumEntry( "Invalid" )

	RT_ENTRY1 = 2,			//:EnumEntry( "Entry1" )
	RT_ENTRY2 = 0x04,		//:EnumEntry( "Entry2" )
	RT_ENTRY3,				//:EnumEntry( "Entry3" )
	RT_ENTRY4				//:EnumEntry( "Entry4" )
};
//:EnumEnd

//:EnumBegin( BITFIELD )
enum EReflectionBitfieldTest
{
	RBT_NONE = 0x0,					//:EnumEntry( "None" )

	RBT_BIT1			= 1 << 0,		//:EnumEntry( "Bit1" )
	RBT_BIT2			= 1 << 1,		//:EnumEntry( "Bit2" )
	RBT_BIT3			= 1 << 2,		//:EnumEntry( "Bit3" )
	RBT_BIT8			= 1 << 7			//:EnumEntry( "Bit8" )
};
//:EnumEnd

//:EnumBegin()
namespace TestNameSpace
{
	enum Test
	{
		NONE = 0,					//:EnumEntry( "None" )

		TEST1,						//:EnumEntry( "Test1" )
		TEST2,						//:EnumEntry( "Test2" )
		TEST3							//:EnumEntry( "Test3" )
	};
}
//:EnumEnd

//:EnumBegin()
enum BaseTest
{
	BT_NONE = 0,					//:EnumEntry( "None" )

	BT_TEST1,						//:EnumEntry( "Test1" )
	BT_TEST2,						//:EnumEntry( "Test2" )
	BT_TEST3,						//:EnumEntry( "Test3" )

	BT_END
};
//:EnumEnd

//:EnumBegin( extends BaseTest )
enum DerivedTest
{
	DT_TEST4 = BT_END,			//:EnumEntry( "Test4" )
	DT_TEST5,						//:EnumEntry( "Test5" )
	DT_TEST6							//:EnumEntry( "Test6" )
};
//:EnumEnd

