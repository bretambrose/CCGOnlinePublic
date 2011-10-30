/**********************************************************************************************************************

	PlatformMutex.h
		Platform-agnostic mutex.  CPP file contains platform-specific implementations.

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
