/**********************************************************************************************************************

	XMLSerializationRegistrar.cpp
		A component containing ??

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

#include "stdafx.h"

#include "XMLSerializationRegistrar.h"

stdext::hash_map< Loki::TypeInfo, XMLSerializerCreationDelegate, STypeInfoContainerHelper > XMLSerializationRegistrar::SerializerCreationDelegateTable;

void XMLSerializationRegistrar::Register_Serializer( const std::type_info &type_id, XMLSerializerCreationDelegate creation_delegate )
{
	Loki::TypeInfo key( type_id );

	FATAL_ASSERT( SerializerCreationDelegateTable.find( key ) == SerializerCreationDelegateTable.end() );
	SerializerCreationDelegateTable[ key ] = creation_delegate;
}

IXMLSerializer *XMLSerializationRegistrar::Create_Serializer( const std::type_info &type_id )
{
	Loki::TypeInfo key( type_id );

	auto iter = SerializerCreationDelegateTable.find( key );
	if ( iter != SerializerCreationDelegateTable.end() )
	{
		return iter->second();
	}

	FATAL_ASSERT( false );

	return nullptr;
}

