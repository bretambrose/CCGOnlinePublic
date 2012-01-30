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


stdext::hash_map< Loki::TypeInfo, IXMLSerializerFactory *, STypeInfoContainerHelper > CXMLSerializationRegistrar::SerializerFactoryTable;

void CXMLSerializationRegistrar::Shutdown( void )
{
	for ( auto iter = SerializerFactoryTable.begin(); iter != SerializerFactoryTable.end(); ++iter )
	{
		delete iter->second;
	}

	SerializerFactoryTable.clear();
}
 
void CXMLSerializationRegistrar::Register_Serializer_Internal( const std::type_info &type_id, IXMLSerializerFactory *factory )
{
	Loki::TypeInfo key( type_id );

	FATAL_ASSERT( SerializerFactoryTable.find( key ) == SerializerFactoryTable.end() );
	SerializerFactoryTable[ key ] = factory;
}

IXMLSerializer *CXMLSerializationRegistrar::Create_Serializer( const std::type_info &type_id )
{
	return Create_Serializer( Loki::TypeInfo( type_id ) );
}

IXMLSerializer *CXMLSerializationRegistrar::Create_Serializer( const Loki::TypeInfo &type_info )
{
	auto iter = SerializerFactoryTable.find( type_info );
	if ( iter != SerializerFactoryTable.end() )
	{
		return iter->second->Create_Serializer();
	}

	FATAL_ASSERT( false );

	return nullptr;
}
