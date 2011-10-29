/**********************************************************************************************************************

	[Placeholder for eventual source license]

	ConcurrentQueueBase.h
		A component defining an abstract base class for concurrent queues.  Several implementations derived from this
		base class, one using Intel's concurrent queue implementation (semi-lockless, uses a spin lock), one using
		a simple mutex to wrap add and remove.

	(c) Copyright 2011, Bret Ambrose.  All rights reserved.

**********************************************************************************************************************/

#ifndef CONCURRENT_QUEUE_BASE_H
#define CONCURRENT_QUEUE_BASE_H

// An abstract base class for concurrent queue implementations
template< typename T >
class IConcurrentQueueBase
{
	public:

		virtual ~IConcurrentQueueBase() {}

		virtual void Add_Item( const T &item ) = 0;
		virtual void Remove_Items( std::vector< T > &items ) = 0;

};

#endif // CONCURRENT_QUEUE_BASE_H
