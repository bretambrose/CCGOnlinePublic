/**********************************************************************************************************************

	[Placeholder for eventual source license]

	PriorityQueueTests.cpp
		defines unit tests for priority queue related functionality

	(c) Copyright 2011, Bret Ambrose.  All rights reserved.

**********************************************************************************************************************/

#include "stdafx.h"

#include "PriorityQueue.h"

void Verify_Sorted( TPriorityQueue< int32 > &int_heap )
{
	if ( int_heap.Empty() )
	{
		return;
	}

	int previous_min;
	int_heap.Peek_Top( previous_min );
	while ( int_heap.Count() > 0 )
	{
		int current_min;
		int_heap.Peek_Top( current_min );

		ASSERT_TRUE( current_min >= previous_min );
		int_heap.Pop();
		previous_min = current_min;
	}
}

TEST( PriorityQueueTests, Constructors )
{
	TPriorityQueue< int32 > int_heap;

	ASSERT_TRUE( int_heap.Count() == 0 );
	ASSERT_TRUE( int_heap.Empty() );

	TPriorityQueue< float > float_heap( 10 );

	ASSERT_TRUE( float_heap.Count() == 0 );
	ASSERT_TRUE( float_heap.Empty() );
}

TEST( PriorityQueueTests, Insert_Pop )
{
	TPriorityQueue< int32 > int_heap;

	int_heap.Insert( 2 );
	int_heap.Insert( 10 );
	int_heap.Insert( 6 );
	int_heap.Insert( 5 );
	int_heap.Insert( 1 );
	int_heap.Insert( 12 );

	int32 min = 0;

	ASSERT_TRUE( int_heap.Peek_Top( min ) );
	ASSERT_TRUE( min == 1 );
	int_heap.Pop();

	ASSERT_TRUE( int_heap.Peek_Top( min ) );
	ASSERT_TRUE( min == 2 );
	int_heap.Pop();

	ASSERT_TRUE( int_heap.Peek_Top( min ) );
	ASSERT_TRUE( min == 5 );
	int_heap.Pop();

	ASSERT_TRUE( int_heap.Peek_Top( min ) );
	ASSERT_TRUE( min == 6 );
	int_heap.Pop();

	ASSERT_TRUE( int_heap.Peek_Top( min ) );
	ASSERT_TRUE( min == 10 );
	int_heap.Pop();

	ASSERT_TRUE( int_heap.Peek_Top( min ) );
	ASSERT_TRUE( min == 12 );
	int_heap.Pop();

	ASSERT_FALSE( int_heap.Peek_Top( min ) );
	ASSERT_TRUE( int_heap.Empty() );
}

TEST( PriorityQueueTests, Insert_Extract_Top )
{
	TPriorityQueue< int32 > int_heap;

	int_heap.Insert( 2 );
	int_heap.Insert( 10 );
	int_heap.Insert( 6 );
	int_heap.Insert( 5 );
	int_heap.Insert( 1 );
	int_heap.Insert( 12 );

	int32 min = 0;

	ASSERT_TRUE( int_heap.Extract_Top( min ) );
	ASSERT_TRUE( min == 1 );

	ASSERT_TRUE( int_heap.Extract_Top( min ) );
	ASSERT_TRUE( min == 2 );

	ASSERT_TRUE( int_heap.Extract_Top( min ) );
	ASSERT_TRUE( min == 5 );

	ASSERT_TRUE( int_heap.Extract_Top( min ) );
	ASSERT_TRUE( min == 6 );

	ASSERT_TRUE( int_heap.Extract_Top( min ) );
	ASSERT_TRUE( min == 10 );

	ASSERT_TRUE( int_heap.Extract_Top( min ) );
	ASSERT_TRUE( min == 12 );

	ASSERT_FALSE( int_heap.Extract_Top( min ) );
	ASSERT_TRUE( int_heap.Empty() );
}

TEST( PriorityQueueTests, Clear )
{
	TPriorityQueue< int32 > int_heap;

	int_heap.Insert( 3 );
	int_heap.Insert( -5 );
	int_heap.Insert( 2 );
	int_heap.Insert( 10 );
	int_heap.Insert( 0 );
	int_heap.Insert( -10 );

	ASSERT_FALSE( int_heap.Empty() );
	ASSERT_TRUE( int_heap.Count() == 6 );

	int_heap.Clear();
	ASSERT_TRUE( int_heap.Empty() );
	ASSERT_TRUE( int_heap.Count() == 0 );
}

TEST( PriorityQueueTests, Remove_By_Index )
{
	TPriorityQueue< int32 > int_heap;

	int_heap.Insert( 1 );
	int_heap.Insert( 10 );
	int_heap.Insert( 2 );
	int_heap.Insert( 20 );
	int_heap.Insert( 30 );
	int_heap.Insert( 3 );

	// Case where Heapify_Push_Up is the pathway that does something
	int_heap.Remove_By_Index( 5 );
	Verify_Sorted( int_heap );

	int_heap.Insert( 1 );
	int_heap.Insert( 10 );
	int_heap.Insert( 2 );
	int_heap.Insert( 20 );
	int_heap.Insert( 30 );
	int_heap.Insert( 3 );

	int_heap.Remove_By_Index( 5 );
	// Case where Heapify_Push_Down is the pathway that does something
	int_heap.Remove_By_Index( 1 );
	Verify_Sorted( int_heap );
}

