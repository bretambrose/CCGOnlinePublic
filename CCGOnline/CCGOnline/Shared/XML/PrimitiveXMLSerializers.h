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

namespace XMLSerializers
{
	IXMLSerializer *Create_Serializer( const int8 &member );
	IXMLSerializer *Create_Serializer( const uint8 &member );
	IXMLSerializer *Create_Serializer( const int16 &member );
	IXMLSerializer *Create_Serializer( const uint16 &member );
	IXMLSerializer *Create_Serializer( const int32 &member );
	IXMLSerializer *Create_Serializer( const uint32 &member );
	IXMLSerializer *Create_Serializer( const int64 &member );
	IXMLSerializer *Create_Serializer( const uint64 &member );
	IXMLSerializer *Create_Serializer( const std::string &member );
	IXMLSerializer *Create_Serializer( const std::wstring &member );
	IXMLSerializer *Create_Serializer( const float &member );
	IXMLSerializer *Create_Serializer( const double &member );
	IXMLSerializer *Create_Serializer( const bool &member );
}

typedef std::pair< uint64, IXMLSerializer * > XMLMemberRecordType;

class COrderedCompositeXMLSerializer : public IXMLSerializer
{
	public:

		COrderedCompositeXMLSerializer( void *dummy_object );
		virtual ~COrderedCompositeXMLSerializer();

		virtual void Load_From_XML( const pugi::xml_node &xml_node, void *destination ) const;

		void Add_Serializer( IXMLSerializer *serializer, const std::wstring &element_name, void *dummy_member );

	private:

		std::vector< std::pair< std::wstring, XMLMemberRecordType > > MemberRecords;

		void *DummyBase;
};

class CUnorderedCompositeXMLSerializer : public IXMLSerializer
{
	public:

		CUnorderedCompositeXMLSerializer( void *dummy_object );
		virtual ~CUnorderedCompositeXMLSerializer();

		virtual void Load_From_XML( const pugi::xml_node &xml_node, void *destination ) const;

		void Add_Serializer( IXMLSerializer *serializer, const std::wstring &element_name, void *dummy_member );
		 
	private:

		stdext::hash_map< std::wstring, XMLMemberRecordType > MemberRecords;

		void *DummyBase;
};

template< typename T >
class CVectorXMLSerializer : public IXMLSerializer
{
	public:

		CVectorXMLSerializer( IXMLSerializer *entry_serializer ) :
			EntrySerializer( entry_serializer )
		{
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
				EntrySerializer->Load_From_XML( iter, &dest[ dest.size() - 1 ] );
			}
		}

	private:

		IXMLSerializer *EntrySerializer;
};

template< typename T >
class CPointerVectorXMLSerializer : public IXMLSerializer
{
	public:

		CPointerVectorXMLSerializer( IXMLSerializer *entry_serializer ) :
			EntrySerializer( entry_serializer )
		{
			FATAL_ASSERT( EntrySerializer != nullptr );
		}

		virtual ~CPointerVectorXMLSerializer()
		{
			if ( EntrySerializer != nullptr )
			{
				delete EntrySerializer;
				EntrySerializer = nullptr;
			}
		}

		virtual void Load_From_XML( const pugi::xml_node &xml_node, void *destination ) const
		{
			std::vector< T * > *dest = reinterpret_cast< std::vector< T * > * >( destination );

			for ( pugi::xml_node iter = xml_node.first_child(); iter; iter = iter.next_sibling() )
			{
				T *object = new T;
				EntrySerializer->Load_From_XML( iter, object );

				dest->push_back( object );
			}
		}

	private:

		IXMLSerializer *EntrySerializer;
};

#endif // PRIMITIVE_XML_SERIALIZERS_H