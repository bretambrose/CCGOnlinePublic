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

#include <cstdint>

#ifdef _DEBUG

#define TBB_USE_DEBUG 1

#endif

#define SAFE_DELETE(x) if((x) != nullptr) { delete (x); }
#define SAFE_DELETE_ARRAY(x) if((x) != nullptr) { delete [](x); }

#define IP_UNREFERENCED_PARAM( x ) ( &reinterpret_cast< const int& >( x ) )

