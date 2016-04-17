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

#include <IPShared/IPShared.h>

#include <IPCore/Debug/DebugAssert.h>

namespace pugi
{
	class xml_node;
}

namespace IP
{
namespace Serialization
{
namespace XML
{

IPSHARED_API class IXMLSerializer
{
	public:

		IXMLSerializer( void ) {}
		virtual ~IXMLSerializer() {}

		virtual void Load_From_XML( const pugi::xml_node &xml_node, void *destination ) const = 0;
		virtual void Load_From_String( const char * /*value*/, void * /*destination*/ ) const { FATAL_ASSERT( false ); }

};

} // namespace XML
} // namespace Serialization
} // namespace IP
