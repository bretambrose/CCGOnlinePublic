/**********************************************************************************************************************

	PlatformMutex.cpp
		Platform-specific implementations of a simple mutex and its helper objects and functions.

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

