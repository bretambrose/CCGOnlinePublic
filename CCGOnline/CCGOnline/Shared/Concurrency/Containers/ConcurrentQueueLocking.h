/**********************************************************************************************************************

	ConcurrentQueueLocking.h
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