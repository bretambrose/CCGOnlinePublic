/**********************************************************************************************************************

	ThreadInterfaces.h
		A component definining the read-only and write-only interfaces that a thread has

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

#ifndef THREAD_INTERFACES_H
#define THREAD_INTERFACES_H

#include "ThreadKey.h"

class CThreadMessageFrame;
template < typename T > class IConcurrentQueueBase;

// The write-only interface to a thread task.  Other threads talk to a thread by writing messages to this
class CWriteOnlyThreadInterface
{
	public:

		CWriteOnlyThreadInterface( const SThreadKey &target_key, const shared_ptr< IConcurrentQueueBase< shared_ptr< CThreadMessageFrame > > > &write_queue );
		~CWriteOnlyThreadInterface();

		const SThreadKey &Get_Target_Key( void ) const { return TargetKey; }

		void Add_Frame( const shared_ptr< CThreadMessageFrame > &frame );

	private:

		SThreadKey TargetKey;

		shared_ptr< IConcurrentQueueBase< shared_ptr< CThreadMessageFrame > > > WriteQueue;

};

// The read-only interface to a thread task.  A thread task processes messages by reading from this
class CReadOnlyThreadInterface
{
	public:

		CReadOnlyThreadInterface( const shared_ptr< IConcurrentQueueBase< shared_ptr< CThreadMessageFrame > > > &read_queue );
		~CReadOnlyThreadInterface();

		void Remove_Frames( std::vector< shared_ptr< CThreadMessageFrame > > &frames );

	private:

		shared_ptr< IConcurrentQueueBase< shared_ptr< CThreadMessageFrame > > > ReadQueue;

};

#endif // THREAD_INTERFACES_H