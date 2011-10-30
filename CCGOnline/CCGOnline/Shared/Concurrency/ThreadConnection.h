/**********************************************************************************************************************

	ThreadConnection.h
		A component definining the class that holds both the read and write interfaces of a thread task.  Only the
		manager has access to this.

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

#ifndef THREAD_CONNECTION_H
#define THREAD_CONNECTION_H

#include "ThreadKey.h"

class CWriteOnlyThreadInterface;
class CReadOnlyThreadInterface;
class CThreadMessageFrame;
template < typename T > class IConcurrentQueueBase;

// A class that holds both the read and write interfaces of a thread task
class CThreadConnection
{
	public:

		CThreadConnection( const SThreadKey &key );
		~CThreadConnection();

		const shared_ptr< CWriteOnlyThreadInterface > &Get_Writer_Interface( void ) const { return WriterInterface; }
		const shared_ptr< CReadOnlyThreadInterface > &Get_Reader_Interface( void ) const { return ReaderInterface; }

		const SThreadKey &Get_Key( void ) const { return Key; }

	private:
		
		SThreadKey Key;

		shared_ptr< IConcurrentQueueBase< shared_ptr< CThreadMessageFrame > > > Queue;

		shared_ptr< CWriteOnlyThreadInterface > WriterInterface;
		shared_ptr< CReadOnlyThreadInterface > ReaderInterface;
};

#endif // THREAD_CONNECTION_H