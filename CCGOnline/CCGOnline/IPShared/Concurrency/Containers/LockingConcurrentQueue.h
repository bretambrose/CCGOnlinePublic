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

#ifndef LOCKING_CONCURRENT_QUEUE_H
#define LOCKING_CONCURRENT_QUEUE_H

#include "ConcurrentQueueInterface.h"

#include "IPPlatform/SynchronizationPrimitives/PlatformMutex.h"

// A concurrent queue that uses a system mutex to guard add and remove operations
template< typename T >
class CLockingConcurrentQueue : public IConcurrentQueue< T >
{
	public:

		// Construction/Destruction
		CLockingConcurrentQueue( void ) :
			Items(),
			Lock( NPlatform::Create_Simple_Mutex() )
		{}

		virtual ~CLockingConcurrentQueue() = default;

		CLockingConcurrentQueue( CLockingConcurrentQueue< T > &&rhs ) = delete;
		CLockingConcurrentQueue< T > & operator =( CLockingConcurrentQueue< T > &&rhs ) = delete;
		CLockingConcurrentQueue( const CLockingConcurrentQueue< T > &rhs ) = delete;
		CLockingConcurrentQueue< T > & operator =( const CLockingConcurrentQueue< T > &rhs ) = delete;

		// Base class public interface implementations
		virtual void Move_Item( T &&item ) override
		{
			CSimplePlatformMutexLocker locker( Lock.get() );

			Items.emplace_back( std::move( item ) );
		}

		virtual void Remove_Items( std::vector< T > &items ) override
		{
			CSimplePlatformMutexLocker locker( Lock.get() );

			items.reserve( items.size() + Items.size() );
			std::move( Items.begin(), Items.end(), std::back_inserter( items ) );

			Items.clear();
		}

	private:

		// Private data
		std::vector< T > Items;

		unique_ptr< ISimplePlatformMutex > Lock;

};

#endif // LOCKING_CONCURRENT_QUEUE_H