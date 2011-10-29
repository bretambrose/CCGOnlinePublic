/**********************************************************************************************************************

	[Placeholder for eventual source license]

	PlatformMutex.h
		Platform-agnostic mutex.  CPP file contains platform-specific implementations.

	(c) Copyright 2011, Bret Ambrose.  All rights reserved.

**********************************************************************************************************************/

#ifndef PLATFORM_MUTEX_H
#define PLATFORM_MUTEX_H

// Pure virtual interface for a simple, platform-agnostic mutex
class ISimplePlatformMutex
{
	public:

		virtual ~ISimplePlatformMutex() {}

		virtual void Acquire( void ) const = 0;
		virtual void Release( void ) const = 0;

};

// Scoped lock helper object that works with the simple platform mutex
class CSimplePlatformMutexLocker
{
	public:

		CSimplePlatformMutexLocker( const ISimplePlatformMutex *mutex );
		~CSimplePlatformMutexLocker();

	private:

		const ISimplePlatformMutex *Mutex;
};

namespace NPlatform
{
	ISimplePlatformMutex *Create_Simple_Mutex( void );
}

#endif // PLATFORM_MUTEX_H
