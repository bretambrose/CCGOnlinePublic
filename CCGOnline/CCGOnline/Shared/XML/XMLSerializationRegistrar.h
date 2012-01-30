/**********************************************************************************************************************

	XMLSerializationRegistrar.h
		??

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

#ifndef XML_SERIALIZATION_REGISTRAR_H
#define XML_SERIALIZATION_REGISTRAR_H

#include "TypeInfoUtils.h"

class IXMLSerializer;

typedef FastDelegate0< IXMLSerializer * > XMLSerializerCreationDelegate;

class IXMLSerializerFactory
{
	public:

		virtual ~IXMLSerializerFactory() {}

		virtual IXMLSerializer *Create_Serializer( void ) const = 0;
};

class CBaseXMLSerializerFactory : public IXMLSerializerFactory
{
	public:

		CBaseXMLSerializerFactory( XMLSerializerCreationDelegate creation_delegate ) :
			CreationDelegate( creation_delegate )
		{}

		virtual IXMLSerializer *Create_Serializer( void ) const 
		{ 
			return CreationDelegate(); 
		}

	private:

		XMLSerializerCreationDelegate CreationDelegate;
};

template < typename T >
class CPointerXMLSerializerFactory : public IXMLSerializerFactory
{
	public:

		CPointerXMLSerializerFactory( XMLSerializerCreationDelegate creation_delegate ) :
			CreationDelegate( creation_delegate )
		{}

		virtual IXMLSerializer *Create_Serializer( void ) const 
		{ 
			return new CPointerXMLSerializer< T >( CreationDelegate() ); 
		}

	private:

		XMLSerializerCreationDelegate CreationDelegate;
};

class CXMLSerializationRegistrar
{
	public:

		static void Shutdown( void );

		template< typename T >
		static void Register_Serializer( XMLSerializerCreationDelegate creation_delegate )
		{
			Register_Serializer_Internal( typeid( T ), new CBaseXMLSerializerFactory( creation_delegate ) );
			Register_Serializer_Internal( typeid( T * ), new CPointerXMLSerializerFactory< T >( creation_delegate ) );
		}

		static IXMLSerializer *Create_Serializer( const std::type_info &type_id );
		static IXMLSerializer *Create_Serializer( const Loki::TypeInfo &type_info );

		template< typename T >
		static IXMLSerializer *Create_Serializer( void ) { return Create_Serializer( typeid( T ) ); }

	private:

		static void Register_Serializer_Internal( const std::type_info &type_id, IXMLSerializerFactory *factory );

		static stdext::hash_map< Loki::TypeInfo, IXMLSerializerFactory *, STypeInfoContainerHelper > SerializerFactoryTable;

};

#endif // XML_SERIALIZATION_REGISTRAR_H