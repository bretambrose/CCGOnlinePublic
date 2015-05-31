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

#ifndef TYPE_INFO_UTILS_H
#define TYPE_INFO_UTILS_H

// A helper policy class that allows us to use Loki's TypeInfo class in STL maps and sets.
struct STypeInfoContainerHelper
{
	public:

		enum {
			bucket_size = 4,  
			min_buckets = 8
		}; 

		size_t operator()( const Loki::TypeInfo &key_value ) const {
			return reinterpret_cast< size_t >( reinterpret_cast< const void * >( &key_value.Get() ) ); 
		}

		bool operator()( const Loki::TypeInfo &left, const Loki::TypeInfo &right ) const {
			return left.before( right );
		}
};

#endif // TYPE_INFO_UTILS_H
