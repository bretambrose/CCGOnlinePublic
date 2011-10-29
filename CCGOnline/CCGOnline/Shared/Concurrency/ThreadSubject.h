/**********************************************************************************************************************

	[Placeholder for eventual source license]

	ThreadSubject.h
		A component containing the type enumerating different thread task subjects.  The subject is the high-level
		logical role of the thread.

	(c) Copyright 2011, Bret Ambrose.  All rights reserved.

**********************************************************************************************************************/

#ifndef THREAD_SUBJECT_H
#define THREAD_SUBJECT_H

enum EThreadSubject
{
	TS_INVALID = 0,
	TS_ALL = TS_INVALID,

	TS_CONCURRENCY_MANAGER,
	TS_LOGIC,
	TS_NETWORK_CONNECTION_MANAGER,
	TS_NETWORK_CONNECTION_SET,
	TS_AI,
	TS_UI,
	TS_DATABASE,
	TS_LOGGING,

	TS_COUNT
};

#endif // THREAD_SUBJECT_H
