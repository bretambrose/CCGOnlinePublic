/**********************************************************************************************************************

	LockingConcurrentQueue.h
		A component defining a concurrent queue that uses a system mutex to guard add and remove operations.

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

#include "SynchronizationPrimitives/PlatformMutex.h"

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

		virtual ~CLockingConcurrentQueue() 
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
		CLockingConcurrentQueue( const CLockingConcurrentQueue< T > &rhs );
		CLockingConcurrentQueue< T > & operator =( const CLockingConcurrentQueue< T > &rhs );

		// Private data
		std::vector< T > Items;

		unique_ptr< ISimplePlatformMutex > Lock;

};

#endif // LOCKING_CONCURRENT_QUEUE_H