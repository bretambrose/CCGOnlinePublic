/**********************************************************************************************************************

	PrimitiveXMLSerializers.h
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

#ifndef PRIMITIVE_XML_SERIALIZERS_H
#define PRIMITIVE_XML_SERIALIZERS_H

#include "XMLSerializerInterface.h"
#include "pugixml.h"
#include "StringUtils.h"
#include "EnumConversion.h"

namespace XMLSerialization
{
	void Register_Primitive_Serializers( void );
}

typedef std::pair< uint64, IXMLSerializer * > XMLMemberRecordType;

class CCompositeXMLSerializer : public IXMLSerializer
{
	public:

		typedef IXMLSerializer BASECLASS;

		CCompositeXMLSerializer( void ) :
			BASECLASS()
		{}

		template< typename T, typename S >
		void Add( const std::wstring &element_name, S T::* dummy_member )
		{
			std::wstring upper_name;
			NStringUtils::To_Upper_Case( element_name, upper_name );

			T dummy_object;
			S *member_pointer = &( dummy_object.*dummy_member );

			uint64 offset = reinterpret_cast< char * >( member_pointer ) - reinterpret_cast< char * >( &dummy_object );

			Add_Member_Record( upper_name,  XMLMemberRecordType( offset, CXMLSerializationRegistrar::Create_Serializer( typeid( S ) ) ) );
		}

		template< typename T, typename S >
		void Add( const std::wstring &element_name, S T::* dummy_member, IXMLSerializer *serializer )
		{
			std::wstring upper_name;
			NStringUtils::To_Upper_Case( element_name, upper_name );

			T dummy_object;
			S *member_pointer = &( dummy_object.*dummy_member );

			uint64 offset = reinterpret_cast< char * >( member_pointer ) - reinterpret_cast< char * >( &dummy_object );

			Add_Member_Record( upper_name,  XMLMemberRecordType( offset, serializer ) );
		}

	protected:

		virtual void Add_Member_Record( const std::wstring &member_name, const XMLMemberRecordType &member_record ) = 0;
};

class COrderedCompositeXMLSerializer : public CCompositeXMLSerializer
{
	public:

		typedef CCompositeXMLSerializer BASECLASS;

		COrderedCompositeXMLSerializer( void ) :
			BASECLASS(),
			MemberRecords()
		{}

		virtual ~COrderedCompositeXMLSerializer()
		{
			for ( auto iter = MemberRecords.begin(); iter != MemberRecords.end(); ++iter )
			{
				delete iter->second.second;
			}

			MemberRecords.clear();
		}

		virtual void Load_From_XML( const pugi::xml_node &xml_node, void *destination ) const
		{
			uint32 member_index = 0;
			uint8 *byte_base_ptr = reinterpret_cast< uint8 * >( destination );

			for ( pugi::xml_node iter = xml_node.first_child(); iter; iter = iter.next_sibling() )
			{
				FATAL_ASSERT( member_index < MemberRecords.size() );

				std::wstring node_name( iter.name() );
				std::wstring upper_name;
				NStringUtils::To_Upper_Case( node_name, upper_name );

				for ( ; member_index < MemberRecords.size() && upper_name != MemberRecords[ member_index ].first; member_index++ )
				{
					;
				}

				FATAL_ASSERT( member_index < MemberRecords.size() );

				IXMLSerializer *serializer = MemberRecords[ member_index ].second.second;
				uint8 *member_ptr = byte_base_ptr + MemberRecords[ member_index ].second.first;

				serializer->Load_From_XML( iter, member_ptr );

				member_index++;
			}

		}

	protected:

		virtual void Add_Member_Record( const std::wstring &member_name, const XMLMemberRecordType &member_record )
		{
			MemberRecords.push_back( std::make_pair( member_name, member_record ) );
		}

	private:

		std::vector< std::pair< std::wstring, XMLMemberRecordType > > MemberRecords;
};


class CUnorderedCompositeXMLSerializer : public CCompositeXMLSerializer
{
	public:

		typedef CCompositeXMLSerializer BASECLASS;

		CUnorderedCompositeXMLSerializer( void ) :
			BASECLASS(),
			MemberRecords()
		{}

		virtual ~CUnorderedCompositeXMLSerializer()
		{
			for ( auto iter = MemberRecords.begin(); iter != MemberRecords.end(); ++iter )
			{
				delete iter->second.second;
			}

			MemberRecords.clear();
		}

		virtual void Load_From_XML( const pugi::xml_node &xml_node, void *destination ) const
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

	protected:

		virtual void Add_Member_Record( const std::wstring &member_name, const XMLMemberRecordType &member_record )
		{
			FATAL_ASSERT( MemberRecords.find( member_name ) == MemberRecords.end() );

			MemberRecords[ member_name ] = member_record;
		}
				 
	private:

		stdext::hash_map< std::wstring, XMLMemberRecordType > MemberRecords;

};

template< typename T >
class CVectorXMLSerializer : public IXMLSerializer
{
	public:

		CVectorXMLSerializer( IXMLSerializer *entry_serializer = nullptr ) :
			EntrySerializer( entry_serializer )
		{
			if ( EntrySerializer == nullptr )
			{
				EntrySerializer = CXMLSerializationRegistrar::Create_Serializer( typeid( T ) );
			}

			FATAL_ASSERT( EntrySerializer != nullptr );
		}

		virtual ~CVectorXMLSerializer()
		{
			if ( EntrySerializer != nullptr )
			{
				delete EntrySerializer;
				EntrySerializer = nullptr;
			}
		}

		virtual void Load_From_XML( const pugi::xml_node &xml_node, void *destination ) const
		{
			std::vector< T > *dest = reinterpret_cast< std::vector< T > * >( destination );

			for ( pugi::xml_node iter = xml_node.first_child(); iter; iter = iter.next_sibling() )
			{
				dest->push_back( T() );
				EntrySerializer->Load_From_XML( iter, &( *dest )[ dest->size() - 1 ] );
			}
		}

	private:

		IXMLSerializer *EntrySerializer;
};

template< typename T >
class CPointerXMLSerializer : public IXMLSerializer
{
	public:

		CPointerXMLSerializer( IXMLSerializer *t_serializer ) :
			TSerializer( t_serializer )
		{
			FATAL_ASSERT( TSerializer != nullptr );
		}

		virtual ~CPointerXMLSerializer()
		{
			if ( TSerializer != nullptr )
			{
				delete TSerializer;
				TSerializer = nullptr;
			}
		}

		virtual void Load_From_XML( const pugi::xml_node &xml_node, void *destination ) const
		{
			T **dest = reinterpret_cast< T ** >( destination );
			*dest = new T;

			TSerializer->Load_From_XML( xml_node, *dest );
		}

	private:

		IXMLSerializer *TSerializer;
};

template< typename T >
class CEnumXMLSerializer : public IXMLSerializer
{
	public:

		CEnumXMLSerializer( void ) {}

		virtual ~CEnumXMLSerializer() {}

		virtual void Load_From_XML( const pugi::xml_node &xml_node, void *destination ) const
		{
			T *dest = reinterpret_cast< T * >( destination );

			if ( !CEnumConverter::Convert( xml_node.child_value(), *dest ) )
			{
				FATAL_ASSERT( false );
			}
		}

};

template< typename T >
class CEnumPointerXMLSerializer : public IXMLSerializer
{
	public:

		CEnumPointerXMLSerializer( void ) {}

		virtual ~CEnumPointerXMLSerializer() {}

		virtual void Load_From_XML( const pugi::xml_node &xml_node, void *destination ) const
		{
			T **dest = reinterpret_cast< T ** >( destination );
			*dest = new T;

			if ( !CEnumConverter::Convert( xml_node.child_value(), **dest ) )
			{
				FATAL_ASSERT( false );
			}
		}

};

template< typename T >
class CEnumPolymorphicXMLSerializer : public IXMLSerializer
{
	public:

		CEnumPolymorphicXMLSerializer( void ) :
			Serializers()
		{
		}

		virtual ~CEnumPolymorphicXMLSerializer()
		{
			for ( auto iter = Serializers.begin(); iter != Serializers.end(); ++iter )
			{
				delete iter->second;
			}

			Serializers.clear();
		}

		virtual void Load_From_XML( const pugi::xml_node &xml_node, void *destination ) const
		{
			pugi::xml_attribute attrib = xml_node.attribute( L"Type" );

			T type_value;
			std::string attribute_value;
			NStringUtils::WideString_To_String( attrib.value(), attribute_value );

			if ( !CEnumConverter::Convert( attribute_value, type_value ) )
			{
				FATAL_ASSERT( false );
			}

			IXMLSerializer *serializer = Serializers.find( type_value )->second;

			serializer->Load_From_XML( xml_node, destination );
		}

		void Add( T key, IXMLSerializer *serializer )
		{
			FATAL_ASSERT( Serializers.find( key ) == Serializers.end() );

			Serializers[ key ] = serializer;
		}

	private:

		stdext::hash_map< T, IXMLSerializer * > Serializers;
};



#endif // PRIMITIVE_XML_SERIALIZERS_H