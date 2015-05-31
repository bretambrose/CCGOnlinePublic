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

#ifndef XML_SERIALIZER_INTERFACE_H
#define XML_SERIALIZER_INTERFACE_H

namespace pugi
{
	class xml_node;
}

class IXMLSerializer
{
	public:

		IXMLSerializer( void ) {}
		virtual ~IXMLSerializer() {}

		virtual void Load_From_XML( const pugi::xml_node &xml_node, void *destination ) const = 0;
		virtual void Load_From_String( const wchar_t * /*value*/, void * /*destination*/ ) const { FATAL_ASSERT( false ); }

};

#endif // XML_SERIALIZER_INTERFACE_H
