/**********************************************************************************************************************

	[Placeholder for eventual source license]

	LoggingMessages.cpp
		A component containing definitions for thread messages needed to log information to files

	(c) Copyright 2011, Bret Ambrose.  All rights reserved.

**********************************************************************************************************************/

#include "stdafx.h"

#include "LoggingMessages.h"

#include "PlatformTime.h"

/**********************************************************************************************************************
	CLogRequestMessage::CLogRequestMessage -- constructor
	
		key -- key of the thread making a log request
		message -- what to write to the thread's log file
		
**********************************************************************************************************************/
CLogRequestMessage::CLogRequestMessage( const SThreadKey &source_key, const std::wstring &message ) :
	SourceKey( source_key ),
	Message( message ),
	Time( CPlatformTime::Get_Raw_Time() )
{
}

