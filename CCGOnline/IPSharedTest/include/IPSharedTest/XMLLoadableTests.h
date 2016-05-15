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

#include <IPCore/Always.h>
 
//:EnumBegin()
enum EPolySerializerTestTypes
{
	PSTT_INVALID = 0,		//:EnumEntry( "Invalid" )

	PSTT_CLASS1 = 1,		//:EnumEntry( "CPolyDerived1" )
	PSTT_CLASS2 = 2		//:EnumEntry( "CPolyDerived2" )

};
//:EnumEnd

//:EnumBegin()
enum ETableTestClass
{
	ETTC_INVALID = 0,		

	ETTC_BERSERKER,		//:EnumEntry( "Berserker" )
	ETTC_JANITOR,			//:EnumEntry( "Janitor" )
	ETTC_BARD				//:EnumEntry( "Bard" )
};
//:EnumEnd

