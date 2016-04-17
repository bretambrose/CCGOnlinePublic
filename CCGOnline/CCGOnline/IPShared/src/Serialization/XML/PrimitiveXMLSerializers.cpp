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

#include <IPShared/Serialization/XML/PrimitiveXMLSerializers.h>

#include <IPShared/Serialization/SerializationRegistrar.h>
#include <pugixml/pugixml.h>

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

		virtual void Load_From_String( const char *value, void *destination ) const override
		{
			int64_t node_value = 0;
			bool result = IP::StringUtils::Convert_Raw( value, node_value );
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

		virtual void Load_From_String( const char *value, void *destination ) const override
		{
			uint64_t node_value = 0;
			bool result = IP::StringUtils::Convert_Raw( value, node_value );
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

		virtual void Load_From_String( const char *value, void *destination ) const override
		{
			double node_value = 0;
			bool result = IP::StringUtils::Convert_Raw( value, node_value );
			FATAL_ASSERT( result );

			T *dest_ptr = reinterpret_cast< T * >( destination );
			*dest_ptr = static_cast< T >( node_value );
		}
};

// A serializer for IP::String
class CXMLStringSerializer : public CPrimitiveXMLSerializer
{
	public:
		
		CXMLStringSerializer( void ) {}
		virtual ~CXMLStringSerializer() = default;

		virtual void Load_From_String( const char *value, void *destination ) const override
		{
			IP::String *dest_ptr = reinterpret_cast< IP::String * >( destination );
			*dest_ptr = value;
		}
};

// A serializer for a boolean data member
class CXMLBoolSerializer : public CPrimitiveXMLSerializer
{
	public:
		
		CXMLBoolSerializer( void ) {}
		virtual ~CXMLBoolSerializer() = default;

		virtual void Load_From_String( const char *value, void *destination ) const override
		{
			bool node_value = false;
			bool result = IP::StringUtils::Convert_Raw( value, node_value );
			FATAL_ASSERT( result );

			bool *dest_ptr = reinterpret_cast< bool * >( destination );
			*dest_ptr = node_value;
		}
};


void Register_Primitive_Serializers( void )
{
	REGISTER_PRIMITIVE_XML_SERIALIZER( int8_t, IP::New< CXMLIntegerSerializer< int8_t > >( MEMORY_TAG ) );
	REGISTER_PRIMITIVE_XML_SERIALIZER( uint8_t, IP::New< CXMLIntegerSerializer< uint8_t > >( MEMORY_TAG ) );
	REGISTER_PRIMITIVE_XML_SERIALIZER( int16_t, IP::New< CXMLIntegerSerializer< int16_t > >( MEMORY_TAG ) );
	REGISTER_PRIMITIVE_XML_SERIALIZER( uint16_t, IP::New< CXMLIntegerSerializer< uint16_t > >( MEMORY_TAG ) );
	REGISTER_PRIMITIVE_XML_SERIALIZER( int32_t, IP::New< CXMLIntegerSerializer< int32_t > >( MEMORY_TAG ) );
	REGISTER_PRIMITIVE_XML_SERIALIZER( uint32_t, IP::New< CXMLIntegerSerializer< uint32_t > >( MEMORY_TAG ) );
	REGISTER_PRIMITIVE_XML_SERIALIZER( int64_t, IP::New< CXMLIntegerSerializer< int64_t > >( MEMORY_TAG ) );
	REGISTER_PRIMITIVE_XML_SERIALIZER( uint64_t, IP::New< CXMLIntegerSerializer< uint64_t > >( MEMORY_TAG ) );
	REGISTER_PRIMITIVE_XML_SERIALIZER( IP::String, IP::New< CXMLStringSerializer >( MEMORY_TAG ) );
	REGISTER_PRIMITIVE_XML_SERIALIZER( double, IP::New< CXMLDoubleSerializer< double > >( MEMORY_TAG ) );
	REGISTER_PRIMITIVE_XML_SERIALIZER( float, IP::New< CXMLDoubleSerializer< float > >( MEMORY_TAG ) );
	REGISTER_PRIMITIVE_XML_SERIALIZER( bool, IP::New< CXMLBoolSerializer >( MEMORY_TAG ) );
}

} // namespace XML
} // namespace Serialization
} // namespace IP
