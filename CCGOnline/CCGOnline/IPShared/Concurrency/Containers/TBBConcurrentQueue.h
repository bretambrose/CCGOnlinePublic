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

#include "ConcurrentQueueInterface.h"

#include "tbb/include/tbb/concurrent_queue.h"

namespace IP
{
namespace Concurrency
{

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

		virtual ~CTBBConcurrentQueue() = default;

		CTBBConcurrentQueue( CTBBConcurrentQueue< T > &&rhs ) = delete;
		CTBBConcurrentQueue< T > & operator =( CTBBConcurrentQueue< T > &&rhs ) = delete;
		CTBBConcurrentQueue( const CTBBConcurrentQueue< T > &rhs ) = delete;
		CTBBConcurrentQueue< T > & operator =( const CTBBConcurrentQueue< T > &rhs ) = delete;

		// Base class public pure virtual interface implementations
		virtual void Move_Item( T &&item ) override
		{
			 Queue.push(std::move(item));
		}

		virtual void Remove_Items( std::vector< T > &items ) override
		{
			items.clear();

			T item;
			while ( Queue.try_pop( item ) )
			{
				items.push_back( std::move( item ) );
			}
		}

	private:

		// Private Data
		tbb::strict_ppl::concurrent_queue< T > Queue;
};

} // namespace Concurrency
} // namespace IP