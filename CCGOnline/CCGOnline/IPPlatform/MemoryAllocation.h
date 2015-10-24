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

//#define USE_CACHE_ALIGNED_ALLOCATION

#pragma warning( push )
#pragma warning( disable : 4290 )

void *operator new( std::size_t size ) throw( std::bad_alloc );
void *operator new( std::size_t size, const std::nothrow_t &no_throw ) throw();
void *operator new[]( std::size_t size ) throw( std::bad_alloc );
void *operator new[]( std::size_t size, const std::nothrow_t &no_throw ) throw();

void operator delete( void *memory ) throw();
void operator delete( void *memory, const std::nothrow_t &no_throw ) throw();
void operator delete[]( void *memory ) throw();
void operator delete[]( void *memory, const std::nothrow_t &no_throw ) throw();

#pragma warning( pop ) 


