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
		CSerializationRegistrar::Register_Primitive_XML_Serializer< int8 >( new CXMLIntegerSerializer< int8 > );
		CSerializationRegistrar::Register_Primitive_XML_Serializer< uint8 >( new CXMLUnsignedIntegerSerializer< uint8 > );
		CSerializationRegistrar::Register_Primitive_XML_Serializer< int16 >( new CXMLIntegerSerializer< int16 > );
		CSerializationRegistrar::Register_Primitive_XML_Serializer< uint16 >( new CXMLUnsignedIntegerSerializer< uint16 > );
		CSerializationRegistrar::Register_Primitive_XML_Serializer< int32 >( new CXMLIntegerSerializer< int32 > );
		CSerializationRegistrar::Register_Primitive_XML_Serializer< uint32 >( new CXMLUnsignedIntegerSerializer< uint32 > );
		CSerializationRegistrar::Register_Primitive_XML_Serializer< int64 >( new CXMLIntegerSerializer< int64 > );
		CSerializationRegistrar::Register_Primitive_XML_Serializer< uint64 >( new CXMLUnsignedIntegerSerializer< uint64 > );	
		CSerializationRegistrar::Register_Primitive_XML_Serializer< std::wstring >( new CXMLWideStringSerializer );
		CSerializationRegistrar::Register_Primitive_XML_Serializer< std::string >( new CXMLStringSerializer );
		CSerializationRegistrar::Register_Primitive_XML_Serializer< double >( new CXMLDoubleSerializer< double > );
		CSerializationRegistrar::Register_Primitive_XML_Serializer< float >( new CXMLDoubleSerializer< float > );
		CSerializationRegistrar::Register_Primitive_XML_Serializer< bool >( new CXMLBoolSerializer );
	}
}
