/**********************************************************************************************************************

	XMLLoadableManager.h
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

#ifndef XML_LOADABLE_MANAGER_H
#define XML_LOADABLE_MANAGER_H

#include "XMLSerializerInterface.h"

typedef FastDelegate1< const pugi::xml_node &, bool > XMLSerializerPredicate;

template< typename T, typename K = ??, typename KeyExtractor = ?? >
class CXMLLoadableTableManager
{
	public:

		CXMLLoadableTableManager( void ) :
			CustomSerializers(),
			Loadables()
		{
		}

		~CXMLLoadableTableManager()
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
			??;
		}

		void Load( file_name ) 
		{
			??;
		}

		void Load( xml_blob ) 
		{
			??;
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

		std::vector< std::pair< XMLSerializerPredicate, ?? > > CustomSerializers;

		stdext::hash_map< K, const T * > Loadables;
};


#endif // XML_LOADABLE_MANAGER_H