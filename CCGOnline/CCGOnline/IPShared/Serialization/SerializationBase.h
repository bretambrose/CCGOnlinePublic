/**********************************************************************************************************************

	SerializationBase.h
		...

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

#ifndef SERIALIZATION_BASE_H
#define SERIALIZATION_BASE_H

#include <functional>

using DPrepDestinationForRead = std::function< void *(void *) >;

template< typename T >
void *Prep_Vector_For_Read( void *destination )
{
	std::vector< T > *dest = reinterpret_cast< std::vector< T > * >( destination );
	
	dest->push_back( T() );
	return static_cast< void * >( &( *dest )[ dest->size() - 1 ] );
}

template< typename T >
void *Prep_Pointer_For_Read( void *destination )
{
	T **dest = reinterpret_cast< T ** >( destination );
	
	*dest = new T;

	return static_cast< void * >( *dest );
}

#endif // SERIALIZATION_BASE_H
