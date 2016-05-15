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

#include <IPShared/Logging/LoggingProcess.h>

#include <IPCore/Debug/DebugAssert.h>
#include <IPCore/Memory/Stl/StringStream.h>
#include <IPCore/Process/Process.h>
#include <IPShared/Concurrency/Messaging/LoggingMessages.h>
#include <IPShared/Concurrency/ProcessExecutionContext.h>
#include <IPShared/EnumConversion.h>
#include <IPShared/Logging/LogInterface.h>
#include <IPShared/MessageHandling/ProcessMessageHandler.h>

#include <fstream>

using namespace IP::Enum;
using namespace IP::Logging;
using namespace IP::Time;

namespace IP
{
namespace Execution
{

// An internal class representing a single, open, active log file connected to the current process
class CLogFile
{
	public:

		CLogFile( EProcessSubject::Enum subject, const IP::String &file_name ) :
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
			File = IP::New< std::ofstream >( MEMORY_TAG, FileName.c_str(), std::ios_base::out | std::ios_base::trunc );
		}

		void Shutdown( void )
		{
			if ( File != nullptr )
			{
				File->close();

				IP::Delete( File );
				File = nullptr;
			}
		}

		void Append_Logging_Message( const IP::String &data )
		{
			FATAL_ASSERT( File != nullptr );

			( *File ) << data;
		}

		EProcessSubject::Enum Get_Subject( void ) const { return Subject; }

		const IP::String &Get_File_Name( void ) const { return FileName; }

		std::ofstream *Get_File( void ) const { return File; }

	private:

		EProcessSubject::Enum Subject;

		IP::String FileName;

		std::ofstream *File;
};


CLoggingProcess::CLoggingProcess( const SProcessProperties &properties ) :
	BASECLASS( properties ),
	LogFiles(),
	PID( IP::Process::Get_Self_PID() ),
	IsShuttingDown( false )
{
}


CLoggingProcess::~CLoggingProcess()
{
	Shutdown();
}


void CLoggingProcess::Initialize( EProcessID id )
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

	REGISTER_THIS_HANDLER( Messaging::CLogRequestMessage, CLoggingProcess, Handle_Log_Request_Message );
}


void CLoggingProcess::Handle_Log_Request_Message( EProcessID source_process_id, IP::UniquePtr< const  Messaging::CLogRequestMessage > &message )
{
	Handle_Log_Request_Message_Aux( source_process_id, message->Get_Source_Properties(), message->Get_Message(), message->Get_Time() );
}


void CLoggingProcess::Handle_Log_Request_Message_Aux( EProcessID source_process_id, const SProcessProperties &properties, const IP::String &message, SystemTimePoint system_time )
{
	if ( IsShuttingDown )
	{
		return;
	}

	// Find the appropriate log file to write to; if one does not exist, create it
	EProcessSubject::Enum subject = static_cast< EProcessSubject::Enum >( properties.Get_Subject() );
	CLogFile *log_file = Get_Log_File( subject );
	if ( log_file == nullptr )
	{
		auto file = IP::Make_Unique< CLogFile >( MEMORY_TAG, subject, Build_File_Name( subject ) );
		file->Initialize();
		log_file = file.get();
		LogFiles.insert( LogFileTableType::value_type( subject, std::move( file ) ) );
	}

	FATAL_ASSERT( log_file != nullptr );

	log_file->Append_Logging_Message( Build_Log_Message( source_process_id, properties, message, system_time ) );
}


IP::String CLoggingProcess::Build_File_Name( EProcessSubject::Enum subject ) const
{
	IP::String subject_string;
	CEnumConverter::Convert( subject, subject_string );

	IP::OStringStream file_name_string;
	file_name_string << "Logs\\" << IP::Logging::CLogInterface::Get_Service_Name() << "_" << PID << "_" << subject_string.c_str() << ".txt";

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


IP::String CLoggingProcess::Build_Log_Message( EProcessID source_process_id, const SProcessProperties &source_properties, const IP::String &message, SystemTimePoint system_time ) const
{
	uint16_t subject_part = source_properties.Get_Subject();
	uint16_t major_part = source_properties.Get_Major_Part();
	uint16_t minor_part = source_properties.Get_Minor_Part();
	uint16_t mode_part = source_properties.Get_Mode_Part();

	IP::String subject_string;
	CEnumConverter::Convert( subject_part, subject_string );

	IP::OStringStream output_string;
	output_string << "[ " << Format_System_Time( system_time ) << " ]( " << static_cast< uint64_t >( source_process_id ) << ": " << subject_string.c_str() << ", " << major_part << ", " << minor_part << ", " << mode_part << " ) : " << message << "\n";

	return output_string.rdbuf()->str();
}


void CLoggingProcess::On_Shutdown_Self_Request( void )
{
	BASECLASS::On_Shutdown_Self_Request();

	IsShuttingDown = true;
}

void CLoggingProcess::Log( IP::String &&message )
{
	Handle_Log_Request_Message_Aux( Get_ID(), Get_Properties(), message, Get_Current_System_Time() );
}

} // namespace Execution
} // namespace IP