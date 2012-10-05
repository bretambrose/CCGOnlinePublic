/**********************************************************************************************************************

	LoggingVirtualProcess.h
		A component defining a virtual process that performs logging of information to files split by thread key

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

#include "LoggingProcess.h"

#include <fstream>
#include <sstream>
#include <iostream>

#include "Concurrency/ProcessSubject.h"
#include "Concurrency/ProcessConstants.h"
#include "Concurrency/Messaging/LoggingMessages.h"
#include "Concurrency/ProcessExecutionContext.h"
#include "MessageHandling/ProcessMessageHandler.h"
#include "Logging/LogInterface.h"
#include "PlatformTime.h"
#include "PlatformProcess.h"
#include "Time/TimeType.h"

// An internal class representing a single, open, active log file connected to the current process
class CLogFile
{
	public:

		CLogFile( EProcessSubject::Enum subject, const std::wstring &file_name ) :
			Subject( subject ),
			FileName( file_name ),
			File( nullptr )
		{}

		~CLogFile()
		{
			Shutdown();
		}

		void Initialize( void )
		{
			File = new std::basic_ofstream< wchar_t >( FileName.c_str(), std::ios_base::out | std::ios_base::trunc );
		}

		void Shutdown( void )
		{
			if ( File != nullptr )
			{
				File->close();

				delete File;
				File = nullptr;
			}
		}

		void Append_Logging_Message( const std::wstring &data )
		{
			FATAL_ASSERT( File != nullptr );

			( *File ) << data;
		}

		EProcessSubject::Enum Get_Subject( void ) const { return Subject; }

		const std::wstring &Get_File_Name( void ) const { return FileName; }

		std::basic_ofstream< wchar_t > *Get_File( void ) const { return File; }

	private:

		EProcessSubject::Enum Subject;

		std::wstring FileName;

		std::basic_ofstream< wchar_t > *File;
};

/**********************************************************************************************************************
	CLoggingProcess::CLoggingProcess -- default constructor
	
		properties -- process properties for the logging process
			
**********************************************************************************************************************/
CLoggingProcess::CLoggingProcess( const SProcessProperties &properties ) :
	BASECLASS( properties ),
	LogFiles(),
	PID( NPlatform::Get_Self_PID() ),
	IsShuttingDown( false )
{
}

/**********************************************************************************************************************
	CLoggingProcess::~CLoggingProcess -- destructor
		
**********************************************************************************************************************/
CLoggingProcess::~CLoggingProcess()
{
	Shutdown();
}

/**********************************************************************************************************************
	CLoggingThreadTask::Initialize -- initializes the process
		
**********************************************************************************************************************/
void CLoggingProcess::Initialize( EProcessID::Enum id )
{
	BASECLASS::Initialize( id );
}

/**********************************************************************************************************************
	CLoggingThreadTask::Service -- primary execution function for this process

		elapsed_seconds -- how much time has elapsed, in seconds
		context -- the thread task execution context
		
**********************************************************************************************************************/
void CLoggingProcess::Service( double elapsed_seconds, const CProcessExecutionContext &context )
{
	// Did we get called directly by the exception handler?
	if ( context.Get_Spawning_Task() == nullptr )
	{
		IsShuttingDown = true;
	}

	BASECLASS::Service( elapsed_seconds, context );

	if ( IsShuttingDown )
	{
		Shutdown();
	}
}

/**********************************************************************************************************************
	CLoggingProcess::Shutdown -- flushes and closes all the log files
		
**********************************************************************************************************************/
void CLoggingProcess::Shutdown( void )
{
	for ( auto iter = LogFiles.cbegin(); iter != LogFiles.cend(); ++iter )
	{
		iter->second->Shutdown();
	}

	LogFiles.clear();
}

/**********************************************************************************************************************
	CLoggingProcess::Register_Message_Handlers -- registers message handlers specific to this process
		
**********************************************************************************************************************/
void CLoggingProcess::Register_Message_Handlers( void )
{
	BASECLASS::Register_Message_Handlers();

	REGISTER_THIS_HANDLER( CLogRequestMessage, CLoggingProcess, Handle_Log_Request_Message );
}

/**********************************************************************************************************************
	CLoggingProcess::Handle_Log_Request_Message -- handler function for all logging requests

		source_process_id -- the process source of the message
		message -- the log request message
		
**********************************************************************************************************************/
void CLoggingProcess::Handle_Log_Request_Message( EProcessID::Enum source_process_id, const shared_ptr< const CLogRequestMessage > &message )
{
	Handle_Log_Request_Message_Aux( source_process_id, message->Get_Source_Properties(), message->Get_Message(), message->Get_Time() );
}

/**********************************************************************************************************************
	CLoggingProcess::Handle_Log_Request_Message_Aux -- handler function for all logging requests, including self
		ones

		source_process_id -- the process source of the message
		subject -- the subject part of the source process properties
		message -- the text to be written to the log file
		
**********************************************************************************************************************/
void CLoggingProcess::Handle_Log_Request_Message_Aux( EProcessID::Enum source_process_id, const SProcessProperties &properties, const std::wstring &message, uint64 system_time )
{
	if ( IsShuttingDown )
	{
		return;
	}

	// Find the appropriate log file to write to; if one does not exist, create it
	EProcessSubject::Enum subject = properties.Get_Subject_As< EProcessSubject::Enum >();
	shared_ptr< CLogFile > log_file = Get_Log_File( subject );
	if ( log_file.get() == nullptr )
	{
		log_file.reset( new CLogFile( subject, Build_File_Name( subject ) ) );

		log_file->Initialize();

		LogFiles[ subject ] = log_file;
	}

	FATAL_ASSERT( log_file.get() != nullptr );

	log_file->Append_Logging_Message( Build_Log_Message( source_process_id, properties, message, system_time ) );
}

/**********************************************************************************************************************
	CLoggingProcess::Build_File_Name -- constructs the log file name to use for the supplied thread subject

		subject -- thread subject of the thread task that sent a logging request

		Returns: the file name to use for a given thread subject
		
**********************************************************************************************************************/
std::wstring CLoggingProcess::Build_File_Name( EProcessSubject::Enum subject ) const
{
#ifdef TOFIX
	static std::wstring _subject_file_names[ TS_COUNT ] = 
	{
		L"",
		L"ConcurrencyManager",
		L"Logic",
		L"NetworkConnectionManager",
		L"NetworkConnectionSet",
		L"AI",
		L"UI",
		L"Database",
		L"Logging"
	};
#endif // TOFIX

	std::basic_ostringstream< wchar_t > file_name_string;
	file_name_string << L"Logs\\" << CLogInterface::Get_Service_Name() << L"_" << PID << L"_" << static_cast< uint32 >( subject ) << L".txt";

	return file_name_string.rdbuf()->str();
}

/**********************************************************************************************************************
	CLoggingProcess::Get_Log_File -- gets an existing log file for a thread subject, if one exists

		subject -- thread subject to get the log file for

		Returns: pointer to the corresponding log file, or null
		
**********************************************************************************************************************/
shared_ptr< CLogFile > CLoggingProcess::Get_Log_File( EProcessSubject::Enum subject ) const
{
	auto iter = LogFiles.find( subject );
	if ( iter != LogFiles.end() )
	{
		return iter->second;
	}

	return shared_ptr< CLogFile >( nullptr );
}

/**********************************************************************************************************************
	CLoggingProcess::Build_Log_Message -- formats a line of text in response to a logging request

		source_key -- key of the virtual process that sent this logging request
		message -- the text to be logged

		Returns: string containing the fully formatted line of text that should be written to the log file
		
**********************************************************************************************************************/
std::wstring CLoggingProcess::Build_Log_Message( EProcessID::Enum source_process_id, const SProcessProperties &source_properties, const std::wstring &message, uint64 system_time ) const
{
	uint16 subject_part = source_properties.Get_Subject();
	uint16 major_part = source_properties.Get_Major_Part();
	uint16 minor_part = source_properties.Get_Minor_Part();
	uint16 mode_part = source_properties.Get_Mode_Part();

	std::basic_ostringstream< wchar_t > output_string;
	output_string << L"[ " << CPlatformTime::Format_Raw_Time( system_time ) << L" ]( " << source_process_id << L": " << subject_part << L", " << major_part << L", " << minor_part << L", " << mode_part << L" ) : " << message << L"\n";

	return output_string.rdbuf()->str();
}

/**********************************************************************************************************************
	CLoggingProcess::Handle_Shutdown_Self_Request -- override of the shutdown request handler

		key -- key of the virtual process that sent this shutdown request
		message -- the shutdown request itself
		
**********************************************************************************************************************/
void CLoggingProcess::Handle_Shutdown_Self_Request( EProcessID::Enum source_process_id, const shared_ptr< const CShutdownSelfRequest > &message )
{
	BASECLASS::Handle_Shutdown_Self_Request( source_process_id, message );

	IsShuttingDown = true;
}

/**********************************************************************************************************************
	CLoggingProcess::Get_Time_Type -- gets the time type that should drive this process's execution

		Returns: the time type by which this process should be executed
		
**********************************************************************************************************************/
ETimeType CLoggingProcess::Get_Time_Type( void ) const 
{ 
	return TT_REAL_TIME; 
}

/**********************************************************************************************************************
	CLoggingProcess::Log -- logs a message to the logging thread's log file

		message -- message to log
		
**********************************************************************************************************************/
void CLoggingProcess::Log( const std::wstring &message )
{
	Handle_Log_Request_Message_Aux( Get_ID(), Get_Properties(), message, CPlatformTime::Get_Raw_Time() );
}
