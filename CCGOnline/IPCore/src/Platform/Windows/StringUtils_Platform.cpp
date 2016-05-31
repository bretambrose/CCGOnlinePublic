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

#include <IPCore/Utils/StringUtils.h>

namespace IP
{
namespace StringUtils
{

bool Convert_Raw( const char *source, bool &value ) 
{
	value = false;
	if ( _stricmp( source, "TRUE" ) == 0 || _stricmp( source, "YES" ) == 0 || _stricmp( source, "1" ) == 0 )
	{
		value = true;
	}

	return true;
}

} // namespace String
} // namespace IP