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

#include <sstream>

namespace IP
{

typedef std::basic_stringstream< char, std::char_traits< char >, IP::Allocator< char > > StringStream;
typedef std::basic_ostringstream< char, std::char_traits< char >, IP::Allocator< char > > OStringStream;
typedef std::basic_stringbuf< char, std::char_traits< char >, IP::Allocator< char > > StringBuf;

} // namespace IP

