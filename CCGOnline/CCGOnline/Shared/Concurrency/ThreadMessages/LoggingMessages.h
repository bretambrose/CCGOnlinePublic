/**********************************************************************************************************************

	[Placeholder for eventual source license]

	LoggingMessages.h
		A component containing definitions for thread messages needed to log information to files

	(c) Copyright 2011, Bret Ambrose.  All rights reserved.

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
