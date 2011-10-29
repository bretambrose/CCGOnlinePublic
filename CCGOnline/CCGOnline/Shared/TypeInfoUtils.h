/**********************************************************************************************************************

	[Placeholder for eventual source license]

	TypeInfoUtils.h
		A helper policy class that allows us to use Loki's TypeInfo class in STL maps and sets.

	(c) Copyright 2011, Bret Ambrose.  All rights reserved.

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
