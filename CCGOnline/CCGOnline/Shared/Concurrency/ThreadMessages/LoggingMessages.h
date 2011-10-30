/**********************************************************************************************************************

	LoggingMessages.h
		A component containing definitions for thread messages needed to log information to files

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

#ifndef LOGGING_MESSAGES_H
#define LOGGING_MESSAGES_H

#include "ThreadMessage.h"

#include "Concurrency\ThreadKey.h"

// A message asking the logging thread to write some information to a file
class CLogRequestMessage : public IThreadMessage
{
	public:

		CLogRequestMessage( const SThreadKey &source_key, const std::wstring &message );
		virtual ~CLogRequestMessage() {}

		const SThreadKey &Get_Source_Key( void ) const { return SourceKey; }
		const std::wstring &Get_Message( void ) const { return Message; }
		uint64 Get_Time( void ) const { return Time; }

	private:

		SThreadKey SourceKey;

		std::wstring Message;

		uint64 Time;
};

#endif // LOGGING_MESSAGES_H
