/**********************************************************************************************************************

	PriorityQueue.h
		Component containing a generic, heap-based implementation of a priority queue

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

#ifndef PRIORITY_QUEUE_H
#define PRIORITY_QUEUE_H

// Our priority queue includes a movement policy to allow us to optionally add functionality that
// allows a priority queue member to track its own internal location in the heap.  This allows
// us to remove arbitrary items from the heap in O( log N ) time rather than O( N ).  This
// allows us to efficiently support cancellable tasks.

// This default movement policy does nothing to support fast removes
template< typename T >
class CDefaultMovementPolicy
{
	public:

		static void Swap( T &lhs, T &rhs )
		{
			if ( &lhs != &rhs )
			{
				T temp = lhs;
				lhs = rhs;
				rhs = temp;
			}
		}

		// A zero index is used to indicate the object is not actually in the heap
		static void Set_Index( const T & /*object*/, size_t /*index*/ )
		{
		}
};

#pragma warning( push )
#pragma warning( disable : 4127 )

// A generic priority queue implementation using a heap as backing store
// The type T must either support less-than based comparison directly, or a custom comparator can be supplied
// The heap implementation is based on the array-based version in CLRS's Introduction to Algorithms.
template< typename T, typename MovementPolicy = CDefaultMovementPolicy< T >, typename ComparisonFunctor = std::less< T > >
class TPriorityQueue {
	public:

		// Constructor
		TPriorityQueue( uint32_t reserve_size = 0 ) :
			Elements(),
			Comparator()
		{			
			if ( reserve_size != 0 )
			{
				Elements.reserve( reserve_size );
			}

			Elements.push_back( T() );
		}
		
		// Destructor
		~TPriorityQueue()
		{
			Clear();
		}

		// Adds a new item to the priority queue
		void Insert( const T &object )
		{
			Elements.push_back( object );

			size_t index = Count();
			MovementPolicy::Set_Index( object, index );

			Heapify_Push_Up( index );
		}

		// Removes the top item from the priority queue
		void Pop( void )
		{
			// Empty?
			size_t count = Count();
			if ( count < 1 )
			{
				return;
			}

			// Make the last element the first, then push down as appropriate
			MovementPolicy::Swap( Elements[ 1 ], Elements[ count ] );
			MovementPolicy::Set_Index( Elements[ count ], 0 );
			Elements.pop_back();

			Heapify_Push_Down( 1 );
		}
		
		// Removes the top item from the queue, assigning a copy of it to an output parameter
		bool Extract_Top( T &top_object )
		{	
			// Empty?		
			size_t count = Count();
			if ( count < 1 )
			{
				return false;
			}
			
			// copy the top element to the output parameter
			top_object = Elements[ 1 ];

			// Make the last element the first, then push down as appropriate
			MovementPolicy::Swap( Elements[ 1 ], Elements[ count ] );
			MovementPolicy::Set_Index( Elements[ count ], 0 );
			Elements.pop_back();

			Heapify_Push_Down( 1 );
					
			return true;
		}

		// Removes the element at the specified index
		void Remove_By_Index( size_t index )
		{
			size_t count = Count();
			FATAL_ASSERT( index > 0 && index <= count );

			// Swap the last into the removal spot, then push down or up as needed
			MovementPolicy::Swap( Elements[ index ], Elements[ count ] );
			MovementPolicy::Set_Index( Elements[ count ], 0 );
			Elements.pop_back();

			if ( index != count )
			{
				Heapify_Push_Up( index );
				Heapify_Push_Down( index );
			}
		}
		
		// Stores a copy of the top element into an output parameter
		bool Peek_Top( T &top_object ) const 
		{
			if ( Count() < 1 )
			{
				return false;
			}
			
			top_object = Elements[ 1 ];
			return true;
		}
		
		// Clears the priority queue of all elements
		void Clear( void )
		{
			size_t count = Elements.size();
			for ( size_t i = 1; i < count; i++ )
			{
				MovementPolicy::Set_Index( Elements[ i ], 0 );
			}

			Elements.clear();
			Elements.push_back( T() );
		}

		// Checks the queue for elements
		bool Empty( void ) const { return Elements.size() == 1; }

		// How many elements in the queue
		size_t Count( void ) const { return Elements.size() - 1; }
			
	private:

		// Three index helper functions for moving up and down the implicit tree-heap.  See CLRS for further explanation.
		static inline size_t Get_Parent_Index( size_t index ) { return index >> 1; }
		static inline size_t Get_Left_Child_Index( size_t index ) { return index << 1; }
		static inline size_t Get_Right_Child_Index( size_t index ) { return ( index << 1 ) + 1; }

		// Performs a series of swaps moving an element down the implicit heap-tree until the element satisfies
		// the basic heap element ordering invariants
		void Heapify_Push_Down( size_t index )
		{
			size_t current_index = index;
			size_t heap_size = Count();

			while ( true )
			{
				// If we're bigger than either our left or right child, we need to move further down
				size_t left_index = Get_Left_Child_Index( current_index );
				size_t right_index = Get_Right_Child_Index( current_index );
				size_t swap_index = current_index;
				if ( left_index <= heap_size && Comparator( Elements[ left_index ], Elements[ swap_index ] ) )
				{
					swap_index = left_index;
				}
				
				if ( right_index <= heap_size && Comparator( Elements[ right_index ], Elements[ swap_index ] ) )
				{
					swap_index = right_index;
				}

				if ( swap_index == current_index )
				{
					// the element is in a satisfactory spot, stop the exchange iteration
					break;
				}

				MovementPolicy::Swap( Elements[ current_index ], Elements[ swap_index ] );

				current_index = swap_index;
			}
		}

		// Performs a series of swaps moving an element up the implicit heap-tree until the element satisfies
		// the basic heap element ordering invariants.  This is only used by the remove function.
		void Heapify_Push_Up( size_t index )
		{
			size_t current_index = index;
			size_t parent_index = Get_Parent_Index( current_index );

			// while we're smaller than our parent, exchange places
			while ( current_index > 1 && Comparator( Elements[ current_index ], Elements[ parent_index ] ) )
			{
				MovementPolicy::Swap( Elements[ current_index ], Elements[ parent_index ] );
				current_index = parent_index;
				parent_index = Get_Parent_Index( current_index );
			}
		}

		// The set of elements in the heap.  The zeroth element is not used.
		std::vector< T > Elements;
		
		// Custom comparison function used to order the elements.
		ComparisonFunctor Comparator;		
};

#pragma warning( pop )

#endif // PRIORITY_QUEUE_H