/**********************************************************************************************************************

	(c) Copyright 2012, Bret Ambrose (mailto:bretambrose@gmail.com).

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

#ifndef PLATFORM_THREAD_H
#define PLATFORM_THREAD_H

class IPlatformThread;

typedef FastDelegate1< void * > ThreadExecutionFunctionType;

// A class that wraps an OS thread.  
class CPlatformThread
{
	public:

		CPlatformThread( void );
		~CPlatformThread();

		void Create_And_Run( uint64_t stack_size, const ThreadExecutionFunctionType &execution_function, void *run_context );
		void Shutdown( void );

		bool Is_Running( void ) const;

	private:

		unique_ptr< IPlatformThread > ThreadImpl;
};

#endif // PLATFORM_THREAD_H
