/**********************************************************************************************************************

	ConcurrentQueueInterface.h
		A component defining an pure virtual interface for concurrent queues.  Several implementations are derived from this
		base class, one using Intel's concurrent queue implementation (semi-lockless, uses a spin lock), one using
		a simple mutex to wrap add and remove.

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

#ifndef CONCURRENT_QUEUE_INTERFACE_H
#define CONCURRENT_QUEUE_INTERFACE_H

// An abstract base class for concurrent queue implementations
template< typename T >
class IConcurrentQueue
{
	public:

		virtual ~IConcurrentQueue() {}

		virtual void Move_Item( T &&item ) = 0;

		virtual void Remove_Items( std::vector< T > &items ) = 0;

};

#endif // CONCURRENT_QUEUE_INTERFACE_H
