/**********************************************************************************************************************

	ProcessSubject.h
		A component containing the type enumerating different process subjects.  The subject is the high-level
		logical role of the process.

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

#ifndef PROCESS_SUBJECT_H
#define PROCESS_SUBJECT_H

namespace EProcessSubject
{
	enum Enum
	{
		INVALID = 0,

		CONCURRENCY_MANAGER,
		LOGGING,

		NEXT_FREE_VALUE
	};
}


#endif // PROCESS_SUBJECT_H
