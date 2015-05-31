/**********************************************************************************************************************

	PrimitiveXMLSerializers.cpp
		A component containing XML serializer definitions

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

#include "IPShared/Serialization/XML/PrimitiveXMLSerializers.h"
#include "IPShared/Serialization/SerializationRegistrar.h"
#include "pugixml/pugixml.h"

class CPrimitiveXMLSerializer : public IXMLSerializer
{
	public:

		virtual void Load_From_XML( const pugi::xml_node &xml_node, void *destination ) const override
		{
			Load_From_String( xml_node.child_value(), destination );
		}
};

// A serializer for a signed integer type of templatized-width
template < typename T >
class CXMLIntegerSerializer : public CPrimitiveXMLSerializer
{
	public:
		
		CXMLIntegerSerializer( void ) {}
		virtual ~CXMLIntegerSerializer() = default;

		virtual void Load_From_String( const wchar_t *value, void *destination ) const override
		{
			int64 node_value = 0;
			bool result = NStringUtils::Convert_Raw( value, node_value );
			FATAL_ASSERT( result );

			T *dest_ptr = reinterpret_cast< T * >( destination );
			*dest_ptr = static_cast< T >( node_value );
		}
};

// A serializer for an unsigned integer type of templatized-width
template < typename T >
class CXMLUnsignedIntegerSerializer : public CPrimitiveXMLSerializer
{
	public:
		
		CXMLUnsignedIntegerSerializer( void ) {}
		virtual ~CXMLUnsignedIntegerSerializer() = default;

		virtual void Load_From_String( const wchar_t *value, void *destination ) const override
		{
			uint64 node_value = 0;
			bool result = NStringUtils::Convert_Raw( value, node_value );
			FATAL_ASSERT( result );

			T *dest_ptr = reinterpret_cast< T * >( destination );
			*dest_ptr = static_cast< T >( node_value );
		}
};

// A serializer for a floating point type of templatized-width
template < typename T >
class CXMLDoubleSerializer : public CPrimitiveXMLSerializer
{
	public:
		
		CXMLDoubleSerializer( void ) {}
		virtual ~CXMLDoubleSerializer() = default;

		virtual void Load_From_String( const wchar_t *value, void *destination ) const override
		{
			double node_value = 0;
			bool result = NStringUtils::Convert_Raw( value, node_value );
			FATAL_ASSERT( result );

			T *dest_ptr = reinterpret_cast< T * >( destination );
			*dest_ptr = static_cast< T >( node_value );
		}
};

// A serializer for std::wstring
class CXMLWideStringSerializer : public CPrimitiveXMLSerializer
{
	public:
		
		CXMLWideStringSerializer( void ) {}
		virtual ~CXMLWideStringSerializer() = default;

		virtual void Load_From_String( const wchar_t *value, void *destination ) const override
		{
			std::wstring node_value( value );

			std::wstring *dest_ptr = reinterpret_cast< std::wstring * >( destination );
			*dest_ptr = node_value;
		}
};

// A serializer for std::string
class CXMLStringSerializer : public CPrimitiveXMLSerializer
{
	public:
		
		CXMLStringSerializer( void ) {}
		virtual ~CXMLStringSerializer() = default;

		virtual void Load_From_String( const wchar_t *value, void *destination ) const override
		{
			std::string node_value;
			NStringUtils::WideString_To_String( value, node_value );

			std::string *dest_ptr = reinterpret_cast< std::string * >( destination );
			*dest_ptr = node_value;
		}
};

// A serializer for a boolean data member
class CXMLBoolSerializer : public CPrimitiveXMLSerializer
{
	public:
		
		CXMLBoolSerializer( void ) {}
		virtual ~CXMLBoolSerializer() = default;

		virtual void Load_From_String( const wchar_t *value, void *destination ) const override
		{
			bool node_value = false;
			bool result = NStringUtils::Convert_Raw( value, node_value );
			FATAL_ASSERT( result );

			bool *dest_ptr = reinterpret_cast< bool * >( destination );
			*dest_ptr = node_value;
		}
};

namespace XMLSerialization
{

	void Register_Primitive_Serializers( void )
	{
		REGISTER_PRIMITIVE_XML_SERIALIZER( int8, new CXMLIntegerSerializer< int8 > );
		REGISTER_PRIMITIVE_XML_SERIALIZER( uint8, new CXMLIntegerSerializer< uint8 > );
		REGISTER_PRIMITIVE_XML_SERIALIZER( int16, new CXMLIntegerSerializer< int16 > );
		REGISTER_PRIMITIVE_XML_SERIALIZER( uint16, new CXMLIntegerSerializer< uint16 > );
		REGISTER_PRIMITIVE_XML_SERIALIZER( int32, new CXMLIntegerSerializer< int32 > );
		REGISTER_PRIMITIVE_XML_SERIALIZER( uint32, new CXMLIntegerSerializer< uint32 > );
		REGISTER_PRIMITIVE_XML_SERIALIZER( int64, new CXMLIntegerSerializer< int64 > );
		REGISTER_PRIMITIVE_XML_SERIALIZER( uint64, new CXMLIntegerSerializer< uint64 > );
		REGISTER_PRIMITIVE_XML_SERIALIZER( std::wstring, new CXMLWideStringSerializer );
		REGISTER_PRIMITIVE_XML_SERIALIZER( std::string, new CXMLStringSerializer );
		REGISTER_PRIMITIVE_XML_SERIALIZER( double, new CXMLDoubleSerializer< double > );
		REGISTER_PRIMITIVE_XML_SERIALIZER( float, new CXMLDoubleSerializer< float > );
		REGISTER_PRIMITIVE_XML_SERIALIZER( bool, new CXMLBoolSerializer );
	}
}
