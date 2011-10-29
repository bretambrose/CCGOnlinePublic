/**********************************************************************************************************************

	[Placeholder for eventual source license]

	ThreadConstants.h
		A component definining a set of thread-key related constants

	(c) Copyright 2011, Bret Ambrose.  All rights reserved.

**********************************************************************************************************************/

#ifndef THREAD_CONSTANTS_H
#define THREAD_CONSTANTS_H

#include "ThreadSubject.h"
#include "ThreadKey.h"

static const uint16 MINOR_KEY_ALL = 0;
static const uint16 MAJOR_KEY_ALL = 0;
static const uint16 INVALID_SUB_KEY = 0;

static const SThreadKey LOG_THREAD_KEY( TS_LOGGING, 1, 1 );
static const SThreadKey MANAGER_THREAD_KEY( TS_CONCURRENCY_MANAGER, 1, 1 );
static const SThreadKey ALL_THREAD_KEY( TS_ALL, MINOR_KEY_ALL, MAJOR_KEY_ALL );
static const SThreadKey INVALID_THREAD_KEY( 0 );

#endif // THREAD_CONSTANTS_H
