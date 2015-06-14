/**********************************************************************************************************************

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

#include "IPShared/EnumConversion.h"
#include "IPShared/Concurrency/ProcessSubject.h"
#include "IPShared/Concurrency/ProcessConstants.h"
#include "IPShared/Concurrency/Messaging/LoggingMessages.h"
#include "IPShared/Concurrency/ProcessExecutionContext.h"
#include "IPShared/MessageHandling/ProcessMessageHandler.h"
#include "LogInterface.h"
#include "IPPlatform/PlatformTime.h"
#include "IPPlatform/PlatformProcess.h"
#include "IPShared/Time/TimeType.h"

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


CLoggingProcess::CLoggingProcess( const SProcessProperties &properties ) :
	BASECLASS( properties ),
	LogFiles(),
	PID( NPlatform::Get_Self_PID() ),
	IsShuttingDown( false )
{
}


CLoggingProcess::~CLoggingProcess()
{
	Shutdown();
}


void CLoggingProcess::Initialize( EProcessID::Enum id )
{
	BASECLASS::Initialize( id );
}


void CLoggingProcess::Run( const CProcessExecutionContext &context )
{
	// Did we get called directly by the exception handler?
	if ( context.Is_Direct() )
	{
		IsShuttingDown = true;
	}

	BASECLASS::Run( context );

	if ( IsShuttingDown )
	{
		Shutdown();
	}
}


void CLoggingProcess::Shutdown( void )
{
	for ( auto iter = LogFiles.cbegin(), end = LogFiles.cend(); iter != end; ++iter )
	{
		iter->second->Shutdown();
	}

	LogFiles.clear();
}


void CLoggingProcess::Register_Message_Handlers( void )
{
	BASECLASS::Register_Message_Handlers();

	REGISTER_THIS_HANDLER( CLogRequestMessage, CLoggingProcess, Handle_Log_Request_Message );
}


void CLoggingProcess::Handle_Log_Request_Message( EProcessID::Enum source_process_id, std::unique_ptr< const CLogRequestMessage > &message )
{
	Handle_Log_Request_Message_Aux( source_process_id, message->Get_Source_Properties(), message->Get_Message(), message->Get_Time() );
}


void CLoggingProcess::Handle_Log_Request_Message_Aux( EProcessID::Enum source_process_id, const SProcessProperties &properties, const std::wstring &message, uint64_t system_time )
{
	if ( IsShuttingDown )
	{
		return;
	}

	// Find the appropriate log file to write to; if one does not exist, create it
	EProcessSubject::Enum subject = properties.Get_Subject_As< EProcessSubject::Enum >();
	CLogFile *log_file = Get_Log_File( subject );
	if ( log_file == nullptr )
	{
		std::unique_ptr< CLogFile > file( new CLogFile( subject, Build_File_Name( subject ) ) );
		file->Initialize();
		log_file = file.get();
		LogFiles.insert( LogFileTableType::value_type( subject, std::move( file ) ) );
	}

	FATAL_ASSERT( log_file != nullptr );

	log_file->Append_Logging_Message( Build_Log_Message( source_process_id, properties, message, system_time ) );
}


std::wstring CLoggingProcess::Build_File_Name( EProcessSubject::Enum subject ) const
{
	std::wstring subject_string;
	CEnumConverter::Convert( subject, subject_string );

	std::basic_ostringstream< wchar_t > file_name_string;
	file_name_string << L"Logs\\" << CLogInterface::Get_Service_Name() << L"_" << PID << L"_" << subject_string.c_str() << L".txt";

	return file_name_string.rdbuf()->str();
}


CLogFile *CLoggingProcess::Get_Log_File( EProcessSubject::Enum subject ) const
{
	auto iter = LogFiles.find( subject );
	if ( iter != LogFiles.end() )
	{
		return iter->second.get();
	}

	return nullptr;
}


std::wstring CLoggingProcess::Build_Log_Message( EProcessID::Enum source_process_id, const SProcessProperties &source_properties, const std::wstring &message, uint64_t system_time ) const
{
	uint16_t subject_part = source_properties.Get_Subject();
	uint16_t major_part = source_properties.Get_Major_Part();
	uint16_t minor_part = source_properties.Get_Minor_Part();
	uint16_t mode_part = source_properties.Get_Mode_Part();

	std::wstring subject_string;
	CEnumConverter::Convert( subject_part, subject_string );

	std::basic_ostringstream< wchar_t > output_string;
	output_string << L"[ " << CPlatformTime::Format_Raw_Time( system_time ) << L" ]( " << source_process_id << L": " << subject_string.c_str() << L", " << major_part << L", " << minor_part << L", " << mode_part << L" ) : " << message << L"\n";

	return output_string.rdbuf()->str();
}


void CLoggingProcess::On_Shutdown_Self_Request( void )
{
	BASECLASS::On_Shutdown_Self_Request();

	IsShuttingDown = true;
}


ETimeType CLoggingProcess::Get_Time_Type( void ) const 
{ 
	return TT_REAL_TIME; 
}


void CLoggingProcess::Log( std::wstring &&message )
{
	Handle_Log_Request_Message_Aux( Get_ID(), Get_Properties(), message, CPlatformTime::Get_Raw_Time() );
}
