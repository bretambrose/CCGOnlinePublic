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

#include <IPCore/IPCore.h>

#include <IPCore/Memory/Memory.h>

#include <unordered_map>

namespace IP
{

template< typename K, 
			 typename V, 
			 typename H = std::hash< K >, 
			 typename E = std::equal_to< K > >
using UnorderedMap = std::unordered_map< K, V, H, E, IP::Allocator< std::pair< const K, V > > >;

template< typename K, 
			 typename V, 
			 typename H = std::hash< K >, 
			 typename E = std::equal_to< K > >
using UnorderedMultiMap = std::unordered_multimap< K, V, H, E, IP::Allocator< std::pair< const K, V > > >;

} // namespace IP
