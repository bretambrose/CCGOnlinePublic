/**********************************************************************************************************************

	[Placeholder for eventual source license]

	ThreadConnection.h
		A component definining the class that holds both the read and write interfaces of a thread task.  Only the
		manager has access to this.

	(c) Copyright 2011, Bret Ambrose.  All rights reserved.

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