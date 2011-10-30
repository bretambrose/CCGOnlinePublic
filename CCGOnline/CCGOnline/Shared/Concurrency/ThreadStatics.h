/**********************************************************************************************************************

	ThreadStatics.h
		A component containing a static class that manages the thread-local variables that hold handles to
		the executing task and the concurrency manager.

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

#ifndef THREAD_STATICS_H
#define THREAD_STATICS_H

class IThreadTask;
class CConcurrencyManager;

// Static class managing thread-local handles to the executing task and the concurrency manager. 
class CThreadStatics
{
	public:

		static void Initialize( void );
		static void Shutdown( void );

		static void Set_Current_Thread_Task( IThreadTask *thread_task );
		static void Set_Concurrency_Manager( CConcurrencyManager *manager );

		static IThreadTask *Get_Current_Thread_Task( void );
		static CConcurrencyManager *Get_Concurrency_Manager( void );

	private:

		// Thread Local storage indices
		static uint32 ThreadHandle;
		static uint32 ConcurrencyManagerHandle;

		static bool Initialized;
};

#endif // THREAD_STATICS_H