/**********************************************************************************************************************

	[Placeholder for eventual source license]

	LogInterface.h
		A component defining the static interface to the logging system.  This system provides services to record
		arbitrary information to files split by thread key function in a thread-safe manner.

	(c) Copyright 2011, Bret Ambrose.  All rights reserved.

**********************************************************************************************************************/

#ifndef LOG_INTERFACE_H
#define LOG_INTERFACE_H

#define ENABLE_LOGGING

class IManagerThreadTask;
class ISimplePlatformMutex;
class CThreadTaskExecutionContext;

// A type enumerating the different levels of logging.  This level can be changed on the fly so that the process
// naturally records more or less information as desired.
enum ELogLevel
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
		
		// Invokes the log thread
		static void Service_Logging( double current_time, const CThreadTaskExecutionContext &context );

		// Access the current logging level; technically not thread-safe, but doesn't matter
		static void Set_Log_Level( ELogLevel log_level ) { LogLevel = log_level; }
		static ELogLevel Get_Log_Level( void ) { return LogLevel; }

		// Access basic log file path and name information
		static const std::wstring &Get_Service_Name( void ) { return ServiceName; }
		static const std::wstring &Get_Log_Path( void ) { return LogPath; }
		static const std::wstring &Get_Archive_Path( void ) { return ArchivePath; }

		// Functions to actually log information to a file
		static void Log( const std::basic_ostringstream< wchar_t > &message_stream );
		static void Log( const std::wstring &message );
		static void Log( const wchar_t *message );

		static shared_ptr< IManagerThreadTask > Get_Logging_Thread( void ) { return LogThread; }

	private:
		
		static std::wstring ServiceName;

		static ISimplePlatformMutex *LogLock;

		static shared_ptr< IManagerThreadTask > LogThread;

		static std::wstring LogPath;
		static std::wstring LogSubdirectory;
		static std::wstring ArchivePath;
		static std::wstring ArchiveSubdirectory;

		static ELogLevel LogLevel;

		static bool StaticInitialized;
		static bool DynamicInitialized;

};

// Conditional macros for logging
#ifdef ENABLE_LOGGING

#define LOG( log_level, stream_expression ) if ( CLogInterface::Get_Log_Level() >= log_level ) { std::basic_ostringstream< wchar_t > log_stream; log_stream << stream_expression; CLogInterface::Log( log_stream ); }

#else

#define LOG( log_level, stream_expression ) 

#endif // ENABLE_LOGGING


#endif // LOG_INTERFACE_H
