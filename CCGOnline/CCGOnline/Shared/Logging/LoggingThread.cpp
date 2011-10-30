/**********************************************************************************************************************

	LoggingThread.h
		A component defining a thread task that performs logging of information to files split by thread key

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

#include "LoggingThread.h"

#include <fstream>
#include <sstream>
#include <iostream>

#include "Concurrency/ThreadSubject.h"
#include "Concurrency/ThreadConstants.h"
#include "Concurrency/ThreadMessages\LoggingMessages.h"
#include "Concurrency/ThreadTaskExecutionContext.h"
#include "MessageHandling/ThreadMessageHandler.h"
#include "Logging/LogInterface.h"
#include "PlatformTime.h"
#include "PlatformProcess.h"
#include "Time/TimeType.h"

// An internal class representing a single, open, active log file connected to the current process
class CLogFile
{
	public:

		CLogFile( EThreadSubject subject, const std::wstring &file_name ) :
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

		EThreadSubject Get_Subject( void ) const { return Subject; }

		const std::wstring &Get_File_Name( void ) const { return FileName; }

		std::basic_ofstream< wchar_t > *Get_File( void ) const { return File; }

	private:

		EThreadSubject Subject;

		std::wstring FileName;

		std::basic_ofstream< wchar_t > *File;
};

/**********************************************************************************************************************
	CLoggingThreadTask::CLoggingThreadTask -- default constructor
		
**********************************************************************************************************************/
CLoggingThreadTask::CLoggingThreadTask( void ) :
	BASECLASS( LOG_THREAD_KEY ),
	LogFiles(),
	PID( NPlatform::Get_Self_Process_ID() ),
	IsShuttingDown( false )
{
}

/**********************************************************************************************************************
	CLoggingThreadTask::~CLoggingThreadTask -- destructor
		
**********************************************************************************************************************/
CLoggingThreadTask::~CLoggingThreadTask()
{
	Shutdown();
}

/**********************************************************************************************************************
	CLoggingThreadTask::Initialize -- initializes the thread task
		
**********************************************************************************************************************/
void CLoggingThreadTask::Initialize( void )
{
	BASECLASS::Initialize();
}

/**********************************************************************************************************************
	CLoggingThreadTask::Service -- primary execution function for this thread task

		elapsed_seconds -- how much time has elapsed, in seconds
		context -- the thread task execution context
		
**********************************************************************************************************************/
void CLoggingThreadTask::Service( double elapsed_seconds, const CThreadTaskExecutionContext &context )
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
	CLoggingThreadTask::Shutdown -- flushes and closes all the log files
		
**********************************************************************************************************************/
void CLoggingThreadTask::Shutdown( void )
{
	for ( auto iter = LogFiles.cbegin(); iter != LogFiles.cend(); ++iter )
	{
		iter->second->Shutdown();
	}

	LogFiles.clear();
}

/**********************************************************************************************************************
	CLoggingThreadTask::Register_Message_Handlers -- registers thread message handlers specific to this thread task
		
**********************************************************************************************************************/
void CLoggingThreadTask::Register_Message_Handlers( void )
{
	BASECLASS::Register_Message_Handlers();

	REGISTER_THIS_HANDLER( CLogRequestMessage, CLoggingThreadTask, Handle_Log_Request_Message );
}

/**********************************************************************************************************************
	CLoggingThreadTask::Handle_Log_Request_Message -- handler function for all logging requests

		key -- the thread task source of the message
		message -- the log request message
		
**********************************************************************************************************************/
void CLoggingThreadTask::Handle_Log_Request_Message( const SThreadKey &key, const shared_ptr< const CLogRequestMessage > &message )
{
	Handle_Log_Request_Message_Aux( key, message->Get_Message(), message->Get_Time() );
}

/**********************************************************************************************************************
	CLoggingThreadTask::Handle_Log_Request_Message_Aux -- handler function for all logging requests, including self
		ones

		key -- the thread task source of the message
		message -- the text to be written to the log file
		
**********************************************************************************************************************/
void CLoggingThreadTask::Handle_Log_Request_Message_Aux( const SThreadKey &key, const std::wstring &message, uint64 system_time )
{
	if ( IsShuttingDown )
	{
		return;
	}

	FATAL_ASSERT( key.Is_Unique() && key.Is_Valid() );

	EThreadSubject subject = key.Get_Thread_Subject();

	// Find the appropriate log file to write to; if one does not exist, create it
	shared_ptr< CLogFile > log_file = Get_Log_File( subject );
	if ( log_file.get() == nullptr )
	{
		log_file.reset( new CLogFile( subject, Build_File_Name( subject ) ) );

		log_file->Initialize();

		LogFiles[ subject ] = log_file;
	}

	FATAL_ASSERT( log_file.get() != nullptr );

	log_file->Append_Logging_Message( Build_Log_Message( key, message, system_time ) );
}

/**********************************************************************************************************************
	CLoggingThreadTask::Build_File_Name -- constructs the log file name to use for the supplied thread subject

		subject -- thread subject of the thread task that sent a logging request

		Returns: the file name to use for a given thread subject
		
**********************************************************************************************************************/
std::wstring CLoggingThreadTask::Build_File_Name( EThreadSubject subject ) const
{
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

	std::basic_ostringstream< wchar_t > file_name_string;
	file_name_string << L"Logs\\" << CLogInterface::Get_Service_Name() << L"_" << PID << L"_" << _subject_file_names[ subject ] << L".txt";

	return file_name_string.rdbuf()->str();
}

/**********************************************************************************************************************
	CLoggingThreadTask::Get_Log_File -- gets an existing log file for a thread subject, if one exists

		subject -- thread subject to get the log file for

		Returns: pointer to the corresponding log file, or null
		
**********************************************************************************************************************/
shared_ptr< CLogFile > CLoggingThreadTask::Get_Log_File( EThreadSubject subject ) const
{
	auto iter = LogFiles.find( subject );
	if ( iter != LogFiles.end() )
	{
		return iter->second;
	}

	return shared_ptr< CLogFile >( nullptr );
}

/**********************************************************************************************************************
	CLoggingThreadTask::Build_Log_Message -- formats a line of text in response to a logging request

		source_key -- key of the thread task that sent this logging request
		message -- the text to be logged

		Returns: string containing the fully formatted line of text that should be written to the log file
		
**********************************************************************************************************************/
std::wstring CLoggingThreadTask::Build_Log_Message( const SThreadKey &source_key, const std::wstring &message, uint64 system_time ) const
{
	uint16 major_subkey = source_key.Get_Major_Sub_Key();
	uint16 minor_subkey = source_key.Get_Minor_Sub_Key();

	std::basic_ostringstream< wchar_t > output_string;
	output_string << L"[ " << CPlatformTime::Format_Raw_Time( system_time ) << L" ]( " << major_subkey << L", " << minor_subkey << L" ) : " << message << L"\n";

	return output_string.rdbuf()->str();
}

/**********************************************************************************************************************
	CLoggingThreadTask::Handle_Shutdown_Thread_Request -- override of the shutdown request handler

		key -- key of the thread task that sent this shutdown request
		message -- the shutdown request itself
		
**********************************************************************************************************************/
void CLoggingThreadTask::Handle_Shutdown_Thread_Request( const SThreadKey &key, const shared_ptr< const CShutdownThreadRequest > &message )
{
	BASECLASS::Handle_Shutdown_Thread_Request( key, message );

	IsShuttingDown = true;
}

/**********************************************************************************************************************
	CLoggingThreadTask::Get_Time_Type -- gets the time type that should drive this thread's execution

		Returns: the time type by which this thread should be executed
		
**********************************************************************************************************************/
ETimeType CLoggingThreadTask::Get_Time_Type( void ) const 
{ 
	return TT_REAL_TIME; 
}

/**********************************************************************************************************************
	CLoggingThreadTask::Log -- logs a message to the logging thread's log file

		message -- message to log
		
**********************************************************************************************************************/
void CLoggingThreadTask::Log( const std::wstring &message )
{
	Handle_Log_Request_Message_Aux( Get_Key(), message, CPlatformTime::Get_Raw_Time() );
}
