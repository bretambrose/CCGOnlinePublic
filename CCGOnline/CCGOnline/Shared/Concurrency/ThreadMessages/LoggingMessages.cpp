/**********************************************************************************************************************

	LoggingMessages.cpp
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

