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

#include <IPCore/Memory/Stl/String.h>
#include <IPCore/Memory/Stl/Vector.h>

namespace IP
{
namespace FileSystem
{

	// Directory functions
	IPCORE_API bool Directory_Exists( const IP::String &path );
	IPCORE_API bool Create_Directory( const IP::String &path );
	IPCORE_API void Delete_Directory( const IP::String &path );

	// File functions
	IPCORE_API void Enumerate_Matching_Files( const IP::String &pattern, IP::Vector< IP::String > &file_names );

	IPCORE_API bool Delete_File( const IP::String &file_name );

	// Misc file-related
	IPCORE_API IP::String Strip_Path( const IP::String &full_path );

} // namespace FileSystem
} // namespace IP

