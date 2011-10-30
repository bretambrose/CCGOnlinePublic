/**********************************************************************************************************************

	ThreadSubject.h
		A component containing the type enumerating different thread task subjects.  The subject is the high-level
		logical role of the thread.

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
