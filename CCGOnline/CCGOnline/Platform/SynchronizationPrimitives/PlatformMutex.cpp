/**********************************************************************************************************************

	[Placeholder for eventual source license]

	PlatformMutex.cpp
		Platform-specific implementations of a simple mutex and its helper objects and functions.

	(c) Copyright 2011, Bret Ambrose.  All rights reserved.

**********************************************************************************************************************/

#include "stdafx.h"

#include "PlatformMutex.h"

/**********************************************************************************************************************
	CSimplePlatformMutexLocker::CSimplePlatformMutexLocker - constructor for the scoped lock

		mutex -- the mutex that this scoped lock will lock across its lifetime

**********************************************************************************************************************/
CSimplePlatformMutexLocker::CSimplePlatformMutexLocker( const ISimplePlatformMutex *mutex ) :
	Mutex( mutex )
{
	FATAL_ASSERT( Mutex != nullptr );

	Mutex->Acquire();
}

/**********************************************************************************************************************
	CSimplePlatformMutexLocker::~CSimplePlatformMutexLocker - destructor for the scoped lock

**********************************************************************************************************************/
CSimplePlatformMutexLocker::~CSimplePlatformMutexLocker()
{
	Mutex->Release();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#ifdef WIN32

// Windows-specific version of the platform mutex
class CSimpleWin32Mutex : public ISimplePlatformMutex
{
	public:

		typedef ISimplePlatformMutex BASECLASS;

		CSimpleWin32Mutex( void ) :
			Mutex( ::CreateMutex( NULL, false, NULL ) )
		{
		}

		virtual ~CSimpleWin32Mutex() 
		{
			::CloseHandle( Mutex );
		}

		virtual void Acquire( void ) const 
		{
			::WaitForSingleObject( Mutex, INFINITE );
		}

		virtual void Release( void ) const 
		{
			::ReleaseMutex( Mutex );
		}

	private:

		HANDLE Mutex;
};

/**********************************************************************************************************************
	NPlatform::Create_Simple_Mutex - creates an os-specific version of the platform mutex

		Returns: a new instance of a platform mutex

**********************************************************************************************************************/
ISimplePlatformMutex *NPlatform::Create_Simple_Mutex( void )
{
	return new CSimpleWin32Mutex;
}

#endif WIN32

