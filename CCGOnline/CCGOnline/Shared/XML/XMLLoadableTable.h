/**********************************************************************************************************************

	XMLLoadableTable.h
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

#ifndef XML_LOADABLE_TABLE_H
#define XML_LOADABLE_TABLE_H

#include "XMLSerializerInterface.h"
#include "pugixml.h"

typedef FastDelegate1< const pugi::xml_node &, bool > XMLSerializerPredicate;

template< typename T, typename S >
T *Creation_Function( void )
{
	return new S;
}

template< typename T, typename K = uint64 >
class CXMLLoadableTable
{
	public:

		typedef FastDelegate1< T *, K > KeyExtractionDelegate;
		typedef FastDelegate0< T * > LoadableCreationDelegate;

		CXMLLoadableTable( KeyExtractionDelegate key_extractor ) :
			KeyExtractor( key_extractor ),
			CustomSerializers(),
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
			CustomSerializers.clear();
		}

		// implicit constraint: S must derive from T
		template< typename S >
		void Register_Custom_Serializer( XMLSerializerPredicate predicate, FastDelegate0< T * > creation_delegate )
		{
			LoadableCreationDelegate loadable_creator( Creation_Function< T, S > );
			
			CustomSerializers.push_back( std::make_pair( predicate, loadable_creator ) );
		}

		void Load( const std::string &file_name ) 
		{
			pugi::xml_document doc;
			pugi::xml_parse_result result = doc.load_file( file_name );
			FATAL_ASSERT( result == true );

			Load( doc.child( "Objects" ) );
		}

		void Load( const pugi::xml_node &xml_node ) 
		{
			std::hash_map< Loki::TypeInfo, IXMLSerializer *, STypeInfoContainerHelper > serializers;

			for ( pugi::xml_node iter = xml_node.first_child(); iter; iter = iter.next_sibling() )
			{
				T *loadable = nullptr;
				for ( uint32 i = 0; i < CustomSerializers.size(); ++i )
				{
					if ( CustomSerializers[ i ].first( iter ) )
					{
						loadable = ( CustomSerializers[ i ].second )();
						break;
					}
				}

				if ( loadable == nullptr )
				{
					loadable = new T;
				}

				IXMLSerializer *serializer = nullptr;
				Loki::TypeInfo loadable_type( typeof( *loadable ) );
				auto s_iter = serializers.find( loadable_type );
				if ( s_iter == serializers.end() )
				{
					serializer = XMLSerializationRegistrar::Create_Serializer( ?? );
					serializers[ loadable_type ] = serializer;
				}
				else
				{
					serializer = s_iter->second;
				}

				serializer->Load_From_XML( iter, loadable );

				K key = KeyExtractor( loadable );
				FATAL_ASSERT( Loadables.find( key ) == Loadables.end() );

				Loadables[ key ] = loadable;
			}

			for ( auto s_iter = serializers.begin(); s_iter != serializers.end(); ++s_iter )
			{
				delete s_iter->second;
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

	private:

		KeyExtractionDelegate KeyExtractor;

		std::vector< std::pair< XMLSerializerPredicate, FastDelegate0< T * > > > CustomSerializers;

		stdext::hash_map< K, const T * > Loadables;
};


#endif // XML_LOADABLE_TABLE_H