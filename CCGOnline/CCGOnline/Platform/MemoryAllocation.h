/**********************************************************************************************************************

	[Placeholder for eventual source license]

	MemoryAllocation.h
		Declarations for global overrides of new and delete.  This file is probably removable since nothing includes it.

	(c) Copyright 2011, Bret Ambrose.  All rights reserved.

**********************************************************************************************************************/

#ifndef MEMORY_ALLOCATION_H
#define MEMORY_ALLOCATION_H

//#define USE_CACHE_ALIGNED_ALLOCATION

#pragma warning( push )
#pragma warning( disable : 4290 )

void *operator new( std::size_t size ) throw( std::bad_alloc );
void *operator new( std::size_t size, const std::nothrow_t &no_throw ) throw();
void *operator new[]( std::size_t size ) throw( std::bad_alloc );
void *operator new[]( std::size_t size, const std::nothrow_t &no_throw ) throw();

void operator delete( void *memory ) throw();
void operator delete( void *memory, const std::nothrow_t &no_throw ) throw();
void operator delete[]( void *memory ) throw();
void operator delete[]( void *memory, const std::nothrow_t &no_throw ) throw();

#pragma warning( pop ) 

#endif // MEMORY_ALLOCATION_H
