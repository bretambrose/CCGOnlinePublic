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

#pragma once

#define ENABLE_LOGGING

namespace IP
{
namespace Execution
{

class IManagedProcess;
class CProcessExecutionContext;

} // namespace Execution

namespace Logging
{

// A type enumerating the different levels of logging.  This level can be changed on the fly so that the process
// naturally records more or less information as desired.
enum class ELogLevel
{
	LL_LOW,
	LL_MEDIUM,
	LL_HIGH,
	LL_VERY_HIGH
};

// Static interface to the logging system
class CLogInterface
{
	public:

		// Initialization and shut down
		// There is a two-level init process to better support unit testing.  It could probably be consolidated though.
		static void Initialize_Static( const std::wstring &service_name, ELogLevel log_level );
		static void Shutdown_Static( void );

		static void Initialize_Dynamic( bool delete_all_logs );
		static void Shutdown_Dynamic( void );
		
		// Invokes the log process
		static void Service_Logging( const IP::Execution::CProcessExecutionContext &context );

		// Access the current logging level; technically not thread-safe, but doesn't matter
		static void Set_Log_Level( ELogLevel log_level ) { LogLevel = log_level; }
		static ELogLevel Get_Log_Level( void ) { return LogLevel; }

		// Access basic log file path and name information
		static const std::wstring &Get_Service_Name( void ) { return ServiceName; }
		static const std::wstring &Get_Log_Path( void ) { return LogPath; }
		static const std::wstring &Get_Archive_Path( void ) { return ArchivePath; }

		// Functions to actually log information to a file
		static void Log( const std::basic_ostringstream< wchar_t > &message_stream );
		static void Log( std::wstring &message );
		static void Log( std::wstring &&message );
		static void Log( const std::wstring &message );
		static void Log( const wchar_t *message );

		static void Log( const std::basic_ostringstream< char > &message_stream );
		static void Log( const std::string &message );
		static void Log( const char *message );

		static std::shared_ptr< IP::Execution::IManagedProcess > Get_Logging_Process( void ) { return LogProcess; }

	private:
		
		static std::wstring ServiceName;

		static std::mutex LogLock;

		static std::shared_ptr< IP::Execution::IManagedProcess > LogProcess;

		static std::wstring LogPath;
		static std::wstring LogSubdirectory;
		static std::wstring ArchivePath;
		static std::wstring ArchiveSubdirectory;

		static ELogLevel LogLevel;

		static bool StaticInitialized;
		static bool DynamicInitialized;

};

} // namespace Logging
} // namespace IP

// Conditional macros for logging
#ifdef ENABLE_LOGGING

#include <sstream>

#define WLOG( log_level, stream_expression ) if ( IP::Logging::CLogInterface::Get_Log_Level() >= log_level ) { std::basic_ostringstream< wchar_t > log_stream; log_stream << stream_expression; IP::Logging::CLogInterface::Log( log_stream ); }
#define LOG( log_level, stream_expression ) if ( IP::Logging::CLogInterface::Get_Log_Level() >= log_level ) { std::basic_ostringstream< char > log_stream; log_stream << stream_expression; IP::Logging::CLogInterface::Log( log_stream ); }

#else

#define WLOG( log_level, stream_expression )
#define LOG( log_level, stream_expression ) 

#endif // ENABLE_LOGGING


