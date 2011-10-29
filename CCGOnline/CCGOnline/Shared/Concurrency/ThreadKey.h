/**********************************************************************************************************************

	[Placeholder for eventual source license]

	ThreadKey.h
		A component definining the unique key that is bound to each thread task within the concurrency system

	(c) Copyright 2011, Bret Ambrose.  All rights reserved.

**********************************************************************************************************************/

#ifndef THREAD_KEY_H
#define THREAD_KEY_H

enum EThreadSubject;

// A class that defines a unique key that is bound to each thread task within the concurrency system
//
// A thread key is divided into three hierarchical that are combined in a bitwise fashion:
//		subject -- the logical role of the thread task (32 bits)
//		major sub key -- a primary numerical index (16 bits)
//		minor sub key -- a secondary numerical index (16 bits)
//
//	The bitwise layout is:
//	63-----------------------------------------------------------------------------0
//	63--- Subject ---------------------------31---Major Sub Key--15---MinorSubKey--0
//
//	The client applications are free to allocate and provide meaning to the two sub key components
// as they see fit.
//
// 32 bits is overkill for the subject; an additional subdivision into 16/16 would be possible if needed
struct SThreadKey
{
	public:

		// Construction
		explicit SThreadKey( uint64 key ) :
			Key( key )
		{}

		SThreadKey( EThreadSubject subject, uint16 major_index, uint16 minor_index );

		SThreadKey( const SThreadKey &rhs ) :
			Key( rhs.Key )
		{}

		uint64 Get_Key( void ) const { return Key; }

		EThreadSubject Get_Thread_Subject( void ) const 
		{
			return static_cast< EThreadSubject >( Key >> 32 );
		}

		uint16 Get_Major_Sub_Key( void ) const 
		{
			return static_cast< uint16 >( ( Key >> 16 ) & 0xFFFF );
		}

		void Set_Major_Sub_Key( uint16 sub_key )
		{
			Key = ( Key & ( ~0xFFFF0000ULL ) ) | ( static_cast< uint64 >( sub_key ) << 16 );
		}

		uint16 Get_Minor_Sub_Key( void ) const 
		{
			return static_cast< uint16 >( Key & 0xFFFF );
		}

		void Set_Minor_Sub_Key( uint16 sub_key )
		{
			Key = ( Key & ( ~0xFFFFULL ) ) | static_cast< uint64 >( sub_key );
		}

		void Get_Key_Parts( EThreadSubject &subject, uint16 &major_index, uint16 &minor_index ) const
		{
			subject = Get_Thread_Subject();
			major_index = Get_Major_Sub_Key();
			minor_index = Get_Minor_Sub_Key();
		}

		bool Is_Valid( void ) const;

		bool Needs_Sub_Key_Allocation( void ) const;

		static uint64 Build_Key( EThreadSubject subject, uint16 major_index, uint16 minor_index ) 
		{
			return ( static_cast< uint64 >( subject ) << 32 ) | ( static_cast< uint64 >( major_index ) << 16 ) | ( static_cast< uint64 >( minor_index ) );
		}

		bool Matches( const SThreadKey &key ) const;
		bool Is_Unique( void ) const;

	private:

		uint64 Key;
};

inline bool operator ==( const SThreadKey &key1, const SThreadKey &key2 )
{
	return key1.Get_Key() == key2.Get_Key();
}

inline bool operator !=( const SThreadKey &key1, const SThreadKey &key2 )
{
	return key1.Get_Key() != key2.Get_Key();
}

// Helper class that lets thread keys be used as keys in STL sets and maps
struct SThreadKeyContainerHelper
{
	public:

		enum {
			bucket_size = 4,  
			min_buckets = 8
		}; 

#ifdef X64
		size_t operator()( const SThreadKey& key_value ) const {
			return key_value.Get_Key();
		}
#else
		size_t operator()( const SThreadKey& key_value ) const {
			uint64 key = key_value.Get_Key();

			return ( key & 0xFFFFFFFF ) ^ ( key >> 32 ); 
		}
#endif

		bool operator()( const SThreadKey &left, const SThreadKey &right ) const {
			return left.Get_Key() < right.Get_Key();
		}
};

#endif // THREAD_KEY_H