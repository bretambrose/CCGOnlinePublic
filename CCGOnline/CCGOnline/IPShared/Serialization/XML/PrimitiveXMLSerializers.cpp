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

#include "stdafx.h"

#include "IPShared/Serialization/XML/PrimitiveXMLSerializers.h"
#include "IPShared/Serialization/SerializationRegistrar.h"
#include "pugixml/pugixml.h"

namespace IP
{
namespace Serialization
{
namespace XML
{

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
			int64_t node_value = 0;
			bool result = IP::String::Convert_Raw( value, node_value );
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
			uint64_t node_value = 0;
			bool result = IP::String::Convert_Raw( value, node_value );
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
			bool result = IP::String::Convert_Raw( value, node_value );
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
			IP::String::WideString_To_String( value, node_value );

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
			bool result = IP::String::Convert_Raw( value, node_value );
			FATAL_ASSERT( result );

			bool *dest_ptr = reinterpret_cast< bool * >( destination );
			*dest_ptr = node_value;
		}
};


void Register_Primitive_Serializers( void )
{
	REGISTER_PRIMITIVE_XML_SERIALIZER( int8_t, new CXMLIntegerSerializer< int8_t > );
	REGISTER_PRIMITIVE_XML_SERIALIZER( uint8_t, new CXMLIntegerSerializer< uint8_t > );
	REGISTER_PRIMITIVE_XML_SERIALIZER( int16_t, new CXMLIntegerSerializer< int16_t > );
	REGISTER_PRIMITIVE_XML_SERIALIZER( uint16_t, new CXMLIntegerSerializer< uint16_t > );
	REGISTER_PRIMITIVE_XML_SERIALIZER( int32_t, new CXMLIntegerSerializer< int32_t > );
	REGISTER_PRIMITIVE_XML_SERIALIZER( uint32_t, new CXMLIntegerSerializer< uint32_t > );
	REGISTER_PRIMITIVE_XML_SERIALIZER( int64_t, new CXMLIntegerSerializer< int64_t > );
	REGISTER_PRIMITIVE_XML_SERIALIZER( uint64_t, new CXMLIntegerSerializer< uint64_t > );
	REGISTER_PRIMITIVE_XML_SERIALIZER( std::wstring, new CXMLWideStringSerializer );
	REGISTER_PRIMITIVE_XML_SERIALIZER( std::string, new CXMLStringSerializer );
	REGISTER_PRIMITIVE_XML_SERIALIZER( double, new CXMLDoubleSerializer< double > );
	REGISTER_PRIMITIVE_XML_SERIALIZER( float, new CXMLDoubleSerializer< float > );
	REGISTER_PRIMITIVE_XML_SERIALIZER( bool, new CXMLBoolSerializer );
}

} // namespace XML
} // namespace Serialization
} // namespace IP
