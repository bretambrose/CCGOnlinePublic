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

#include <IPShared/IPShared.h>

#include <IPCore/Memory/Stl/UnorderedMap.h>
#include <IPShared/Serialization/SerializationRegistrar.h>
#include <IPShared/Serialization/XML/XMLSerializerInterface.h>
#include <pugixml/pugixml.h>

namespace IP
{
namespace Serialization
{
namespace XML
{

template< typename K, typename T >
class CXMLLoadableTable
{
	public:

		using KeyExtractorMemberFunction = const K & ( T::* )( void ) const ;
		using PostLoadMemberFunction = void ( T::* )( void );
		using TableType = IP::UnorderedMap< K, const T * >;
		using TableIterator = typename TableType::const_iterator;

		CXMLLoadableTable( KeyExtractorMemberFunction key_extractor, const char *top_child = nullptr ) :
			KeyExtractor( key_extractor ),
			PostLoad(),
			TopChildName( top_child ? top_child : "Objects" ),
			Loadables()
		{
		}

		~CXMLLoadableTable()
		{
			std::for_each( Loadables.begin(), Loadables.end(), []( const TableType::value_type &val ) { IP::Delete( val.second ); } );
			Loadables.clear();
		}

		void Load( const IP::String &file_name ) 
		{
			pugi::xml_document doc;
			pugi::xml_parse_result result = doc.load_file( file_name.c_str() );
			FATAL_ASSERT( result == true );

			Load( doc );
		}

		void Load( const pugi::xml_document &xml_doc ) 
		{
			IXMLSerializer *serializer = CSerializationRegistrar::Get_XML_Serializer< T * >();
			FATAL_ASSERT( serializer != nullptr );

			pugi::xml_node top = xml_doc.child( TopChildName.c_str() );
			for ( pugi::xml_node iter = top.first_child(); iter; iter = iter.next_sibling() )
			{
				T *loadable = nullptr;

				serializer->Load_From_XML( iter, &loadable );
				if ( PostLoad != nullptr )
				{
					(loadable->*PostLoad)();
				}

				FATAL_ASSERT( loadable != nullptr );

				K key = (loadable->*KeyExtractor)();
				FATAL_ASSERT( Loadables.find( key ) == Loadables.cend() );

				Loadables[ key ] = loadable;
			}
		}

		const T *Get_Object( const K &key ) const
		{
			auto iter = Loadables.find( key );
			if ( iter != Loadables.cend() )
			{
				return iter->second;
			}

			return nullptr;
		}

		TableIterator cbegin( void ) const { return Loadables.cbegin(); }
		TableIterator cend( void ) const { return Loadables.cend(); }

		void Set_Post_Load_Function( PostLoadMemberFunction post_load ) { PostLoad = post_load; }

	private:

		KeyExtractorMemberFunction KeyExtractor;
		PostLoadMemberFunction PostLoad;

		IP::String TopChildName;

		TableType Loadables;
};

} // namespace XML
} // namespace Serialization
} // namespace IP