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

#include <IPShared/Logging/LogInterface.h>

#include <IPCore/Debug/DebugAssert.h>
#include <IPCore/File/FileSystem.h>
#include <IPCore/System/Time.h>
#include <IPShared/Concurrency/ConcurrencyManager.h>
#include <IPShared/Concurrency/ProcessConstants.h>
#include <IPShared/Concurrency/ProcessStatics.h>
#include <IPShared/Logging/LoggingProcess.h>


using namespace IP::Execution;

namespace IP
{
namespace Logging
{

// Static class data member definitions
std::mutex CLogInterface::LogLock;
std::shared_ptr< IManagedProcess > CLogInterface::LogProcess( nullptr );

ELogLevel CLogInterface::LogLevel( ELogLevel::LL_LOW );
IP::String CLogInterface::ServiceName( "" );

IP::String CLogInterface::LogPath( "Logs\\" );
IP::String CLogInterface::LogSubdirectory( "Logs" );

IP::String CLogInterface::ArchivePath( "Logs\\Archives\\" );
IP::String CLogInterface::ArchiveSubdirectory( "Logs\\Archives" );

bool CLogInterface::StaticInitialized = false;
bool CLogInterface::DynamicInitialized = false;


void CLogInterface::Initialize_Static( const IP::String &service_name, ELogLevel log_level )
{
	FATAL_ASSERT( StaticInitialized == false );

	ServiceName = service_name;
	LogLevel = log_level;

	// Create the logging directory if it does not exist
	if ( !IP::FileSystem::Directory_Exists( LogSubdirectory ) )
	{
		IP::FileSystem::Create_Directory( LogSubdirectory );
	}

	// Create the log archive directory if it does not exist
	if ( !IP::FileSystem::Directory_Exists( ArchiveSubdirectory ) )
	{
		IP::FileSystem::Create_Directory( ArchiveSubdirectory );
	}

	StaticInitialized = true;
}


void CLogInterface::Shutdown_Static( void )
{
	if ( !StaticInitialized )
	{
		return;
	}

	Shutdown_Dynamic();

	StaticInitialized = false;
}


void CLogInterface::Initialize_Dynamic( bool delete_all_logs )
{
	static const uint64_t SECONDS_PER_HOUR = 3600;

	FATAL_ASSERT( StaticInitialized == true );

	std::lock_guard< std::mutex > lock( LogLock );

	FATAL_ASSERT( DynamicInitialized == false );

	// Remove old logs if they exist, logs that are not removable are attached to currently running processes
	IP::OStringStream file_pattern_string;
	file_pattern_string << LogPath << ServiceName << "*.txt";

	IP::Vector< IP::String > matching_file_names;
	IP::FileSystem::Enumerate_Matching_Files( file_pattern_string.rdbuf()->str(), matching_file_names );

	auto current_time = IP::Time::Get_Current_System_Time();

	// Try to remove all matching log files that are older than an hour
	for ( uint32_t i = 0; i < matching_file_names.size(); ++i )
	{
		auto file_time = IP::Time::Get_File_Last_Modified_Time( LogPath + matching_file_names[ i ] );
		auto time_difference = current_time - file_time;

		if ( delete_all_logs || std::chrono::duration_cast< std::chrono::seconds >( time_difference ).count() > SECONDS_PER_HOUR )
		{
			IP::FileSystem::Delete_File( LogPath + matching_file_names[ i ] );
		}
	}

	LogProcess.reset( IP::New< CLoggingProcess >( MEMORY_TAG, LOGGING_PROCESS_PROPERTIES ) );

	DynamicInitialized = true;
}


void CLogInterface::Shutdown_Dynamic( void )
{
	FATAL_ASSERT( StaticInitialized == true );

	if ( DynamicInitialized )
	{
		std::lock_guard< std::mutex > lock( LogLock );

		LogProcess = nullptr;

		DynamicInitialized = false;
	}
}


void CLogInterface::Service_Logging( const IP::Execution::CProcessExecutionContext &context )
{
	std::lock_guard< std::mutex > lock( LogLock );

	if ( LogProcess.get() != nullptr )
	{
		auto old_process = CProcessStatics::Get_Current_Process();

		CProcessStatics::Set_Current_Process( LogProcess.get() );
		LogProcess->Run( context );
		CProcessStatics::Set_Current_Process( old_process );
		LogProcess->Flush_System_Messages();
	}
}


void CLogInterface::Log( IP::String &message )
{
	Log( std::move( message ) );
}


void CLogInterface::Log( const IP::String &message )
{
	IP::String message_copy( message );
	Log( std::move( message_copy ) );
}


void CLogInterface::Log( IP::String &&message )
{
	IProcess *virtual_process = CProcessStatics::Get_Current_Process();
	if ( virtual_process != nullptr )
	{
		virtual_process->Log( std::move( message ) );
		return;
	}

	CConcurrencyManager *manager = CProcessStatics::Get_Concurrency_Manager();
	if ( manager != nullptr )
	{
		manager->Log( std::move( message ) );
	}
}

void CLogInterface::Log( const char *message )
{
	IP::String w_message( message );
	Log( std::move( w_message ) );
}


void CLogInterface::Log( const IP::OStringStream &message_stream )
{
	Log( message_stream.rdbuf()->str() );
}

} // namespace Logging
} // namespace IP