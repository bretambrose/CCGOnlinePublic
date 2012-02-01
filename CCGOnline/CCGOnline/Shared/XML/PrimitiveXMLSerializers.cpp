/**********************************************************************************************************************

	PrimitiveXMLSerializers.cpp
		A component containing 

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

#include "PrimitiveXMLSerializers.h"
#include "XMLSerializationRegistrar.h"
#include "pugixml.h"


template < typename T >
class CXMLIntegerSerializer : public IXMLSerializer
{
	public:
		
		CXMLIntegerSerializer( void ) {}
		virtual ~CXMLIntegerSerializer() {}

		virtual void Load_From_XML( const pugi::xml_node &xml_node, void *destination ) const
		{
			int64 node_value = _wcstoi64( xml_node.child_value(), nullptr, 10 );
			int32 error = errno;
			FATAL_ASSERT( error == 0 );

			T *dest_ptr = reinterpret_cast< T * >( destination );
			*dest_ptr = static_cast< T >( node_value );
		}
};

template < typename T >
class CXMLUnsignedIntegerSerializer : public IXMLSerializer
{
	public:
		
		CXMLUnsignedIntegerSerializer( void ) {}
		virtual ~CXMLUnsignedIntegerSerializer() {}

		virtual void Load_From_XML( const pugi::xml_node &xml_node, void *destination ) const
		{
			uint64 node_value = _wcstoui64( xml_node.child_value(), nullptr, 10 );
			int32 error = errno;
			FATAL_ASSERT( error == 0 );

			T *dest_ptr = reinterpret_cast< T * >( destination );
			*dest_ptr = static_cast< T >( node_value );
		}
};

template < typename T >
class CXMLDoubleSerializer : public IXMLSerializer
{
	public:
		
		CXMLDoubleSerializer( void ) {}
		virtual ~CXMLDoubleSerializer() {}

		virtual void Load_From_XML( const pugi::xml_node &xml_node, void *destination ) const
		{
			double node_value = _wtof( xml_node.child_value() );
			int32 error = errno;
			FATAL_ASSERT( error == 0 );

			T *dest_ptr = reinterpret_cast< T * >( destination );
			*dest_ptr = static_cast< T >( node_value );
		}
};

class CXMLWideStringSerializer : public IXMLSerializer
{
	public:
		
		CXMLWideStringSerializer( void ) {}
		virtual ~CXMLWideStringSerializer() {}

		virtual void Load_From_XML( const pugi::xml_node &xml_node, void *destination ) const
		{
			std::wstring node_value( xml_node.child_value() );

			std::wstring *dest_ptr = reinterpret_cast< std::wstring * >( destination );
			*dest_ptr = node_value;
		}
};

class CXMLStringSerializer : public IXMLSerializer
{
	public:
		
		CXMLStringSerializer( void ) {}
		virtual ~CXMLStringSerializer() {}

		virtual void Load_From_XML( const pugi::xml_node &xml_node, void *destination ) const
		{
			std::string node_value;
			NStringUtils::WideString_To_String( xml_node.child_value(), node_value );

			std::string *dest_ptr = reinterpret_cast< std::string * >( destination );
			*dest_ptr = node_value;
		}
};

class CXMLBoolSerializer : public IXMLSerializer
{
	public:
		
		CXMLBoolSerializer( void ) {}
		virtual ~CXMLBoolSerializer() {}

		virtual void Load_From_XML( const pugi::xml_node &xml_node, void *destination ) const
		{
			std::wstring node_value( xml_node.child_value() );
			bool value = false;
			if ( _wcsnicmp( node_value.c_str(), L"TRUE", node_value.size() ) == 0 || 
				  _wcsnicmp( node_value.c_str(), L"YES", node_value.size() ) == 0 ||
				  _wcsnicmp( node_value.c_str(), L"1", node_value.size() ) == 0 )
			{
				value = true;
			}

			bool *dest_ptr = reinterpret_cast< bool * >( destination );
			*dest_ptr = value;
		}
};

namespace XMLSerialization
{
	IXMLSerializer *Create_Int8_Serializer( void ) { return new CXMLIntegerSerializer< int8 >; }
	IXMLSerializer *Create_UInt8_Serializer( void ) { return new CXMLUnsignedIntegerSerializer< uint8 >; }
	IXMLSerializer *Create_Int16_Serializer( void ) { return new CXMLIntegerSerializer< int16 >; }
	IXMLSerializer *Create_UInt16_Serializer( void ) { return new CXMLUnsignedIntegerSerializer< uint16 >; }
	IXMLSerializer *Create_Int32_Serializer( void ) { return new CXMLIntegerSerializer< int32 >; }
	IXMLSerializer *Create_UInt32_Serializer( void ) { return new CXMLUnsignedIntegerSerializer< uint32 >; }
	IXMLSerializer *Create_Int64_Serializer( void ) { return new CXMLIntegerSerializer< int64 >; }
	IXMLSerializer *Create_UInt64_Serializer( void ) { return new CXMLUnsignedIntegerSerializer< uint64 >; }
	IXMLSerializer *Create_WString_Serializer( void ) { return new CXMLWideStringSerializer; }
	IXMLSerializer *Create_String_Serializer( void ) { return new CXMLStringSerializer; }
	IXMLSerializer *Create_Double_Serializer( void ) { return new CXMLDoubleSerializer< double >; }
	IXMLSerializer *Create_Float_Serializer( void ) { return new CXMLDoubleSerializer< float >; }
	IXMLSerializer *Create_Bool_Serializer( void ) { return new CXMLBoolSerializer; }

	void Register_Primitive_Serializers( void )
	{
		CXMLSerializationRegistrar::Register_Serializer< int8 >( Create_Int8_Serializer );
		CXMLSerializationRegistrar::Register_Serializer< uint8 >( Create_UInt8_Serializer );
		CXMLSerializationRegistrar::Register_Serializer< int16 >( Create_Int16_Serializer );
		CXMLSerializationRegistrar::Register_Serializer< uint16 >( Create_UInt16_Serializer );
		CXMLSerializationRegistrar::Register_Serializer< int32 >( Create_Int32_Serializer );
		CXMLSerializationRegistrar::Register_Serializer< uint32 >( Create_UInt32_Serializer );
		CXMLSerializationRegistrar::Register_Serializer< int64 >( Create_Int64_Serializer );
		CXMLSerializationRegistrar::Register_Serializer< uint64 >( Create_UInt64_Serializer );	
		CXMLSerializationRegistrar::Register_Serializer< std::wstring >( Create_WString_Serializer );
		CXMLSerializationRegistrar::Register_Serializer< std::string >( Create_String_Serializer );
		CXMLSerializationRegistrar::Register_Serializer< double >( Create_Double_Serializer );
		CXMLSerializationRegistrar::Register_Serializer< float >( Create_Float_Serializer );
		CXMLSerializationRegistrar::Register_Serializer< bool >( Create_Bool_Serializer );
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/*

CUnorderedCompositeXMLSerializer::CUnorderedCompositeXMLSerializer( void *dummy_object ) :
	MemberRecords(),
	DummyBase( dummy_object )
{
}

CUnorderedCompositeXMLSerializer::~CUnorderedCompositeXMLSerializer()
{
	for ( auto iter = MemberRecords.begin(); iter != MemberRecords.end(); ++iter )
	{
		delete iter->second.second;
	}

	MemberRecords.clear();
}

void CUnorderedCompositeXMLSerializer::Load_From_XML( const pugi::xml_node &xml_node, void *destination ) const
{
	uint8 *byte_base_ptr = reinterpret_cast< uint8 * >( destination );

	for ( pugi::xml_node iter = xml_node.first_child(); iter; iter = iter.next_sibling() )
	{
		std::wstring node_name( iter.name() );
		std::wstring upper_name;
		NStringUtils::To_Upper_Case( node_name, upper_name );

		auto member_iter = MemberRecords.find( upper_name );
		FATAL_ASSERT( member_iter != MemberRecords.end() );

		IXMLSerializer *serializer = member_iter->second.second;
		uint8 *member_ptr = byte_base_ptr + member_iter->second.first;

		serializer->Load_From_XML( iter, member_ptr );
	}
}

void CUnorderedCompositeXMLSerializer::Add_Serializer( IXMLSerializer *serializer, const std::wstring &element_name, void *dummy_member )
{
	std::wstring upper_name;
	NStringUtils::To_Upper_Case( element_name, upper_name );
	FATAL_ASSERT( MemberRecords.find( upper_name ) == MemberRecords.end() );

	uint8 *base_byte_ptr = reinterpret_cast< uint8 * >( DummyBase );
	uint8 *member_byte_ptr = reinterpret_cast< uint8 * >( dummy_member );

	uint64 offset = member_byte_ptr - base_byte_ptr;

	MemberRecords.insert( std::make_pair< std::wstring, XMLMemberRecordType >( upper_name, XMLMemberRecordType( offset, serializer ) ) );
}

*/