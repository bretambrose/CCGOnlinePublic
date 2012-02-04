/**********************************************************************************************************************

	XMLSerializationRegistrar.h
		A component containing a registrar of xml serializers, indexed by type.

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

// Serializer factories are a simple class hierarchy that lets us delay and compose serialization which we couldn't
// otherwise do with C++ delegates.  For example, given a serializer for type T, by delaying and wrapping in a class
// we can create a serializer (factory) for type T * automatically.  If there's a way of taking a function delegate
// and creating another modified function delegate (ie T serializer creator to T * serializer creator) then I could
// remove this.
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

template < typename T >
class CEnumXMLSerializerFactory : public IXMLSerializerFactory
{
	public:

		CEnumXMLSerializerFactory( void ) {}

		virtual IXMLSerializer *Create_Serializer( void ) const 
		{ 
			return new CEnumXMLSerializer< T >; 
		}

};

template < typename T >
class CEnumPointerXMLSerializerFactory : public IXMLSerializerFactory
{
	public:

		CEnumPointerXMLSerializerFactory( void ) {}

		virtual IXMLSerializer *Create_Serializer( void ) const 
		{ 
			return new CEnumPointerXMLSerializer< T >; 
		}

};

// Tracks creation proxies for all xml serializers
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

		template< typename T >
		static void Register_Enum_Serializer( void )
		{
			Register_Serializer_Internal( typeid( T ), new CEnumXMLSerializerFactory< T > );
			Register_Serializer_Internal( typeid( T * ), new CEnumPointerXMLSerializerFactory< T > );
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