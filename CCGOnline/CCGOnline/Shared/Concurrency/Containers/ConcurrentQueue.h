/**********************************************************************************************************************

	[Placeholder for eventual source license]

	ConcurrentQueue.h
		A component defining a concurrent queue that wraps TBB's concurrent queue.

	(c) Copyright 2011, Bret Ambrose.  All rights reserved.

**********************************************************************************************************************/

#ifndef CONCURRENT_QUEUE_H
#define CONCURRENT_QUEUE_H

#include "ConcurrentQueueBase.h"

#include "tbb/concurrent_queue.h"

// A concurrent queue that wraps TBB's concurrent queue
template< typename T >
class CConcurrentQueue : public IConcurrentQueueBase< T >
{
	public:

		// Construction/Destruction
		CConcurrentQueue( void ) :
			Queue()
		{
		}

		virtual ~CConcurrentQueue() 
		{
		}

		// Base class public pure virtual interface implementations
		virtual void Add_Item( const T &item )
		{
			Queue.push( item );
		}

		virtual void Remove_Items( std::vector< T > &items )
		{
			items.clear();

			T item;
			while ( Queue.try_pop( item ) )
			{
				items.push_back( item );
			}
		}

	private:

		// Do not define, prevent copy and assignment
		CConcurrentQueue( const CConcurrentQueue< T > &rhs );
		CConcurrentQueue< T > & operator =( const CConcurrentQueue< T > &rhs );

		// Private Data
		tbb::strict_ppl::concurrent_queue< T > Queue;
};

#endif // CONCURRENT_QUEUE_H