/**********************************************************************************************************************

	[Placeholder for eventual source license]

	MemoryAllocation.cpp
		Definitions for global overrides of new and delete.  TBB's scalable allocator acts as a replacement.
		The entries here are taken from Chapter 11 (page 257) of the book, Intel Threading Building Blocks.

	(c) Copyright 2011, Bret Ambrose.  All rights reserved.

**********************************************************************************************************************/

#include "stdafx.h"

#include "MemoryAllocation.h"
#include "tbb/scalable_allocator.h"

#pragma warning( push )
#pragma warning( disable : 4290 )

void *operator new( std::size_t size ) throw( std::bad_alloc )
{
	if ( size  == 0 )
	{
		size = 1;
	}

	void *raw_memory = scalable_malloc( size );
	if ( raw_memory != nullptr )
	{
		return raw_memory;
	}

	throw std::bad_alloc();
}

void *operator new( std::size_t size, const std::nothrow_t & /*no_throw*/ ) throw()
{
	if ( size  == 0 )
	{
		size = 1;
	}

	return scalable_malloc( size );
}

void *operator new[]( std::size_t size ) throw( std::bad_alloc )
{
	return operator new( size );
}

void *operator new[]( std::size_t size, const std::nothrow_t &no_throw ) throw()
{
	return operator new( size, no_throw );
}

void operator delete( void *memory ) throw()
{
	if ( memory != nullptr )
	{
		scalable_free( memory );
	}
}

void operator delete( void *memory, const std::nothrow_t & /*no_throw*/ ) throw()
{
	if ( memory != nullptr )
	{
		scalable_free( memory );
	}
}

void operator delete[]( void *memory ) throw()
{
	operator delete( memory );
}

void operator delete[]( void *memory, const std::nothrow_t &no_throw ) throw()
{
	operator delete( memory, no_throw );
}

#pragma warning( pop ) 
