/**********************************************************************************************************************

	[Placeholder for eventual source license]

	ConcurrentQueueLocking.h
		A component defining a concurrent queue that uses a system mutex to guard add and remove operations.

	(c) Copyright 2011, Bret Ambrose.  All rights reserved.

**********************************************************************************************************************/

#ifndef CONCURRENT_QUEUE_LOCKING_H
#define CONCURRENT_QUEUE_LOCKING_H

#include "ConcurrentQueueBase.h"

#include "SynchronizationPrimitives/PlatformMutex.h"

// A concurrent queue that uses a system mutex to guard add and remove operations
template< typename T >
class CConcurrentQueueLocking : public IConcurrentQueueBase< T >
{
	public:

		// Construction/Destruction
		CConcurrentQueueLocking( void ) :
			Items(),
			Lock( NPlatform::Create_Simple_Mutex() )
		{}

		virtual ~CConcurrentQueueLocking() 
		{
		}

		// Base class public interface implementations
		virtual void Add_Item( const T &item )
		{
			CSimplePlatformMutexLocker locker( Lock );

			Items.push_back( item );
		}

		virtual void Remove_Items( std::vector< T, std::allocator< T > > &items )
		{
			CSimplePlatformMutexLocker locker( Lock );

			items = Items;
			Items.clear();
		}

	private:

		// Do not define, prevent copy and assignment
		CConcurrentQueueLocking( const CConcurrentQueueLocking< T > &rhs );
		CConcurrentQueueLocking< T > & operator =( const CConcurrentQueueLocking< T > &rhs );

		// Private data
		std::vector< T > Items;

		scoped_ptr< ISimplePlatformMutex > Lock;

};

#endif // CONCURRENT_QUEUE_LOCKING_H