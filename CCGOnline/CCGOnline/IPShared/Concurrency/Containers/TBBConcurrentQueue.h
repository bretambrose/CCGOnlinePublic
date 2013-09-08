/**********************************************************************************************************************

	TBBConcurrentQueue.h
		A component defining a concurrent queue that wraps TBB's concurrent queue.

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

#ifndef TBB_CONCURRENT_QUEUE_H
#define TBB_CONCURRENT_QUEUE_H

#include "ConcurrentQueueInterface.h"

#include "tbb/include/tbb/concurrent_queue.h"

// A concurrent queue that wraps TBB's concurrent queue
template< typename T >
class CTBBConcurrentQueue : public IConcurrentQueue< T >
{
	public:

		// Construction/Destruction
		CTBBConcurrentQueue( void ) :
			Queue()
		{
		}

		virtual ~CTBBConcurrentQueue() 
		{
		}

		// Base class public pure virtual interface implementations
		virtual void Enqueue_Item( T && /*item*/ )
		{
			 // TBI
		}

		virtual void Enqueue_Item( T & /*item*/ )
		{
			 // TBI
		}

		virtual void Remove_Items( std::vector< T > & /*items*/ )
		{
#ifdef TOFIX
			items.clear();

			T item;
			while ( Queue.try_pop( item ) )
			{
				items.push_back( item );
			}
#endif // TOFIX
		}

	private:

		// Do not define, prevent copy and assignment
		CTBBConcurrentQueue( const CTBBConcurrentQueue< T > &rhs );
		CTBBConcurrentQueue< T > & operator =( const CTBBConcurrentQueue< T > &rhs );

		// Private Data
		tbb::strict_ppl::concurrent_queue< T > Queue;
};

#endif // TBB_CONCURRENT_QUEUE_H