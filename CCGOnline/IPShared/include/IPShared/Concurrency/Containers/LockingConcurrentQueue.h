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

#include <IPShared/IPShared.h>

#include <IPCore/Memory/Stl/Vector.h>
#include <IPShared/Concurrency/Containers/ConcurrentQueueInterface.h>

#include <mutex>

namespace IP
{
namespace Concurrency
{

// A concurrent queue that uses a system mutex to guard add and remove operations
template< typename T >
class CLockingConcurrentQueue : public IConcurrentQueue< T >
{
	public:

		// Construction/Destruction
		CLockingConcurrentQueue( void ) :
			Items(),
			Lock()
		{}

		virtual ~CLockingConcurrentQueue() = default;

		CLockingConcurrentQueue( CLockingConcurrentQueue< T > &&rhs ) = delete;
		CLockingConcurrentQueue< T > & operator =( CLockingConcurrentQueue< T > &&rhs ) = delete;
		CLockingConcurrentQueue( const CLockingConcurrentQueue< T > &rhs ) = delete;
		CLockingConcurrentQueue< T > & operator =( const CLockingConcurrentQueue< T > &rhs ) = delete;

		// Base class public interface implementations
		virtual void Move_Item( T &&item ) override
		{
			std::lock_guard< std::mutex > lock( Lock );

			Items.emplace_back( std::move( item ) );
		}

		virtual void Remove_Items( IP::Vector< T > &items ) override
		{
			std::lock_guard< std::mutex > lock( Lock );

			items.reserve( items.size() + Items.size() );
			std::move( Items.begin(), Items.end(), std::back_inserter( items ) );

			Items.clear();
		}

	private:

		// Private data
		IP::Vector< T > Items;

		std::mutex Lock;

};

} // namespace Concurrency
} // namespace IP