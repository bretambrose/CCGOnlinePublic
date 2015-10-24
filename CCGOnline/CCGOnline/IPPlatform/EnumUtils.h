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

namespace IP
{
namespace Enum
{

// make all functions constexpr when compiler supports
template < typename T >
bool Are_All_Enum_Flags_Set( T value, T mask )
{
	static_assert( std::is_enum< T >::value, "Type T must be an enum" );

	using BaseEnumType = std::underlying_type< T >::type;

	return ( static_cast< BaseEnumType >( value ) & static_cast< BaseEnumType >( mask ) ) == static_cast< BaseEnumType >( mask );
}

template < typename T >
bool Is_An_Enum_Flag_Set( T value, T mask )
{
	static_assert( std::is_enum< T >::value, "Type T must be an enum" );

	using BaseEnumType = std::underlying_type< T >::type;

	return ( static_cast< BaseEnumType >( value ) & static_cast< BaseEnumType >( mask ) ) != 0;
}

template < typename T >
T Make_Enum_Mask( T flag )
{
	static_assert( std::is_enum< T >::value, "Type T must be an enum" );

	return flag;
}

template < typename T, typename... Args >
T Make_Enum_Mask( T first, Args... rest )
{
	static_assert( std::is_enum< T >::value, "Type T must be an enum" );

	using BaseEnumType = std::underlying_type< T >::type;

	BaseEnumType rest_value = static_cast< BaseEnumType >( Make_Enum_Mask( rest... ) );

	return static_cast< T >( static_cast< BaseEnumType >( first ) | rest_value );
}

} // namespace Enum
} // namespace IP
