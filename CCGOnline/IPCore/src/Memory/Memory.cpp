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

#include <IPCore/Memory/Memory.h>

#include <tbb/scalable_allocator.h>

namespace IP
{

void *Malloc( const char* tag, size_t memory_size )
{
	IP_UNREFERENCED_PARAM( tag );

	return scalable_malloc( memory_size );
}

void Free( void *memory_ptr )
{
	scalable_free( memory_ptr );
}

} // namespace IP