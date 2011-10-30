/**********************************************************************************************************************

	ThreadConstants.h
		A component definining a set of thread-key related constants

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
