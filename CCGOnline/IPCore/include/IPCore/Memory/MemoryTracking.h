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

#ifdef TRACK_MEMORY_ALLOCATIONS

#define TOSTRING2(x) #x
#define TOSTRING(x) TOSTRING2( x )
#define MEMORY_TAG __FILE__ ## ":" ## TOSTRING( __LINE__ )

#else

#define MEMORY_TAG nullptr

#endif // TRACK_MEMORY_ALLOCATIONS