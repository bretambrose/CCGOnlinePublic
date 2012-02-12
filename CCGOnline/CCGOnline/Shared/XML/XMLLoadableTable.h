/**********************************************************************************************************************

	XMLLoadableTable.h
		Definition for a container of XML loadable objects keyed by some type/function

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

#ifndef XML_LOADABLE_TABLE_H
#define XML_LOADABLE_TABLE_H

#include "XMLSerializerInterface.h"
#include "pugixml.h"
#include "XMLSerializationRegistrar.h"

template< typename K, typename T >
class CXMLLoadableTable
{
	public:

		//typedef FastDelegate1< const T *, const K & > KeyExtractionDelegate;
		typedef const K & ( T::* KeyExtractorMemberFunction )( void ) const ;
		typedef void ( T::* PostLoadMemberFunction )( void );
		typedef typename stdext::hash_map< K, const T * >::const_iterator TableIterator;

		CXMLLoadableTable( KeyExtractorMemberFunction key_extractor, const wchar_t *top_child = nullptr, IXMLSerializer *serializer = nullptr ) :
			KeyExtractor( key_extractor ),
			PostLoad(),
			TopChildName( top_child ? top_child : L"Objects" ),
			Serializer( serializer ),
			Loadables()
		{
		}

		~CXMLLoadableTable()
		{
			for ( auto iter = Loadables.begin(); iter != Loadables.end(); ++iter )
			{
				delete iter->second;
			}

			Loadables.clear();

			if ( Serializer != nullptr )
			{
				delete Serializer;
				Serializer = nullptr;
			}
		}

		void Load( const std::string &file_name ) 
		{
			pugi::xml_document doc;
			pugi::xml_parse_result result = doc.load_file( file_name.c_str() );
			FATAL_ASSERT( result == true );

			Load( doc );
		}

		void Load( const pugi::xml_document &xml_doc ) 
		{
			IXMLSerializer *serializer = Serializer;
			if ( serializer == nullptr )
			{
				serializer = CXMLSerializationRegistrar::Create_Serializer( typeid( T * ) );
			}

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
				FATAL_ASSERT( Loadables.find( key ) == Loadables.end() );

				Loadables[ key ] = loadable;
			}

			if ( Serializer == nullptr )
			{
				delete serializer;
			}
		}

		const T *Get_Object( const K &key ) const
		{
			auto iter = Loadables.find( key );
			if ( iter != Loadables.end() )
			{
				return iter->second;
			}

			return nullptr;
		}

		void Add_Object( T *object )
		{
			K key = object->Get_Key();
			FATAL_ASSERT( Loadables.find( key ) == Loadables.end() );

			Loadables[ key ] = object;
		}

		TableIterator cbegin( void ) const { return Loadables.cbegin(); }
		TableIterator cend( void ) const { return Loadables.cend(); }

		void Set_Post_Load_Function( PostLoadMemberFunction post_load ) { PostLoad = post_load; }

	private:

		KeyExtractorMemberFunction KeyExtractor;
		PostLoadMemberFunction PostLoad;

		std::wstring TopChildName;

		IXMLSerializer *Serializer;

		stdext::hash_map< K, const T * > Loadables;
};


#endif // XML_LOADABLE_TABLE_H