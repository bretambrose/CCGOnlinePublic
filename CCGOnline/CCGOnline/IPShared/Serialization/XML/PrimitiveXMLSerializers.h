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

#pragma once

#include "IPShared/Serialization/XML/XMLSerializerInterface.h"
#include "pugixml/pugixml.h"
#include "IPPlatform/StringUtils.h"
#include "IPShared/EnumConversion.h"
#include "IPShared/Serialization/SerializationHelpers.h"

namespace IP
{
namespace Serialization
{
namespace XML
{

void Register_Primitive_Serializers( void );

using XMLMemberRecordType = std::pair< uint64_t, IXMLSerializer * >;

// The base class for the serializer for compound types: classes and structs
class CCompositeXMLSerializer : public IXMLSerializer
{
	public:

		using BASECLASS = IXMLSerializer;

		CCompositeXMLSerializer( void ) :
			BASECLASS(),
			MemberRecords()
		{}

		virtual ~CCompositeXMLSerializer() = default;

		virtual void Load_From_XML( const pugi::xml_node &xml_node, void *destination ) const override
		{
			uint8_t *byte_base_ptr = reinterpret_cast< uint8_t * >( destination );

			for ( pugi::xml_node iter = xml_node.first_child(); iter; iter = iter.next_sibling() )
			{
				std::wstring node_name( iter.name() );
				std::wstring upper_name;
				IP::String::To_Upper_Case( node_name, upper_name );

				auto member_iter = MemberRecords.find( upper_name );
				FATAL_ASSERT( member_iter != MemberRecords.cend() );

				IXMLSerializer *serializer = member_iter->second.second;
				uint8_t *member_ptr = byte_base_ptr + member_iter->second.first;

				serializer->Load_From_XML( iter, member_ptr );
			}

			for ( pugi::xml_attribute att_iter = xml_node.first_attribute(); att_iter; att_iter = att_iter.next_attribute() )
			{
				std::wstring attribute_name( att_iter.name() );
				std::wstring upper_attribute_name;
				IP::String::To_Upper_Case( attribute_name, upper_attribute_name );

				auto record_iter = MemberRecords.find( upper_attribute_name );
				if ( record_iter == MemberRecords.cend() )
				{
					if ( upper_attribute_name == L"TYPE" )
					{
						continue;
					}

					FATAL_ASSERT( false );
				}

				IXMLSerializer *serializer = record_iter->second.second;
				uint8_t *member_ptr = byte_base_ptr + record_iter->second.first;

				serializer->Load_From_String( att_iter.value(), member_ptr );
			}
		}

		void Add( const std::wstring &element_name, uint64_t offset, IXMLSerializer *serializer )
		{
			std::wstring upper_name;
			IP::String::To_Upper_Case( element_name, upper_name );

			Add_Member_Record( upper_name,  XMLMemberRecordType( offset, serializer ) );
		}

	private:

		virtual void Add_Member_Record( const std::wstring &member_name, const XMLMemberRecordType &member_record )
		{
			FATAL_ASSERT( MemberRecords.find( member_name ) == MemberRecords.cend() );

			MemberRecords[ member_name ] = member_record;
		}

		using MemberRecordTableType = std::unordered_map< std::wstring, XMLMemberRecordType >;
		MemberRecordTableType MemberRecords;
};

// A serializer for a std::vector of some type
class CVectorXMLSerializer : public IXMLSerializer
{
	public:

		CVectorXMLSerializer( IXMLSerializer *entry_serializer, DPrepDestinationForRead prep_delegate ) :
			EntrySerializer( entry_serializer ),
			PrepDelegate( prep_delegate )
		{
			FATAL_ASSERT( EntrySerializer != nullptr );
			FATAL_ASSERT( PrepDelegate != false );
		}

		template< typename T >
		static IXMLSerializer* Create( IXMLSerializer *t_serializer )
		{
			return new CVectorXMLSerializer( t_serializer, Prep_Vector_For_Read< T > );
		}

		virtual ~CVectorXMLSerializer() = default;

		virtual void Load_From_XML( const pugi::xml_node &xml_node, void *destination ) const override
		{
			for ( pugi::xml_node iter = xml_node.first_child(); iter; iter = iter.next_sibling() )
			{
				EntrySerializer->Load_From_XML( iter, PrepDelegate( destination ) );
			}
		}

	private:

		IXMLSerializer *EntrySerializer;
		DPrepDestinationForRead PrepDelegate;
};

// A serializer for a pointer to some type
class CPointerXMLSerializer : public IXMLSerializer
{
	public:

		CPointerXMLSerializer( IXMLSerializer *t_serializer, DPrepDestinationForRead prep_delegate ) :
			TSerializer( t_serializer ),
			PrepDelegate( prep_delegate )
		{
			FATAL_ASSERT( TSerializer != nullptr );
			FATAL_ASSERT( PrepDelegate != false );
		}

		template< typename T >
		static IXMLSerializer* Create( IXMLSerializer *t_serializer )
		{
			return new CPointerXMLSerializer( t_serializer, Prep_Pointer_For_Read< T > );
		}

		virtual ~CPointerXMLSerializer() = default;

		virtual void Load_From_XML( const pugi::xml_node &xml_node, void *destination ) const override
		{
			TSerializer->Load_From_XML( xml_node, PrepDelegate( destination ) );
		}

	private:

		IXMLSerializer *TSerializer;
		DPrepDestinationForRead PrepDelegate;
};

// A serializer for an enum
template< typename T >
class CEnumXMLSerializer : public IXMLSerializer
{
	public:

		CEnumXMLSerializer( void ) {}

		virtual ~CEnumXMLSerializer() = default;

		virtual void Load_From_XML( const pugi::xml_node &xml_node, void *destination ) const override
		{
			Load_From_String( xml_node.child_value(), destination );
		}

		virtual void Load_From_String( const wchar_t *value, void *destination ) const override
		{
			T *dest = reinterpret_cast< T * >( destination );

			if ( !IP::Enum::CEnumConverter::Convert( value, *dest ) )
			{
				FATAL_ASSERT( false );
			}
		}
};

// A serializer for the base class of a class hierarchy, where all leaves of the hierarchy have a corresponding enum entry
class CEnumPolymorphicXMLSerializer : public IXMLSerializer
{
	public:

		CEnumPolymorphicXMLSerializer( const Loki::TypeInfo &enum_type_info ) :
			EnumTypeInfo( enum_type_info ),
			Serializers()
		{
		}

		virtual ~CEnumPolymorphicXMLSerializer() = default;

		virtual void Load_From_XML( const pugi::xml_node &xml_node, void *destination ) const override
		{
			pugi::xml_attribute attrib = xml_node.attribute( L"Type" );

			uint64_t type_value;
			std::string attribute_value;

			if ( !IP::Enum::CEnumConverter::Convert( EnumTypeInfo, std::wstring( attrib.value() ), type_value ) )
			{
				FATAL_ASSERT( false );
			}

			IXMLSerializer *serializer = Serializers.find( type_value )->second;

			serializer->Load_From_XML( xml_node, destination );
		}

		void Add( uint64_t key, IXMLSerializer *serializer )
		{
			FATAL_ASSERT( Serializers.find( key ) == Serializers.cend() );

			Serializers[ key ] = serializer;
		}

	private:

		using SerializerTableType = std::unordered_map< uint64_t, IXMLSerializer * >;
		
		Loki::TypeInfo EnumTypeInfo;

		SerializerTableType Serializers;
};



} // namespace XML
} // namespace Serialization
} // namespace IP