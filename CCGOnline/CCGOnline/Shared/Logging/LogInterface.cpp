/**********************************************************************************************************************

	[Placeholder for eventual source license]

	LogInterface.cpp
		A component defining the static interface to the logging system.  This system provides services to record
		arbitrary information to files split by thread key function in a thread-safe manner.

	(c) Copyright 2011, Bret Ambrose.  All rights reserved.

**********************************************************************************************************************/

#include "stdafx.h"

#include "LogInterface.h"

#include <sstream>

#include "LoggingThread.h"
#include "Concurrency/ThreadStatics.h"
#include "Concurrency/ThreadTaskInterface.h"
#include "Concurrency/ConcurrencyManager.h"
#include "PlatformTime.h"
#include "PlatformFileSystem.h"
#include "SynchronizationPrimitives/PlatformMutex.h"

// Static class data member definitions
ISimplePlatformMutex *CLogInterface::LogLock( nullptr );
shared_ptr< IManagerThreadTask > CLogInterface::LogThread( nullptr );

ELogLevel CLogInterface::LogLevel( LL_LOW );
std::wstring CLogInterface::ServiceName( L"" );

std::wstring CLogInterface::LogPath( L"Logs\\" );
std::wstring CLogInterface::LogSubdirectory( L"Logs" );

std::wstring CLogInterface::ArchivePath( L"Logs\\Archives\\" );
std::wstring CLogInterface::ArchiveSubdirectory( L"Logs\\Archives" );

bool CLogInterface::StaticInitialized = false;
bool CLogInterface::DynamicInitialized = false;

/**********************************************************************************************************************
	CLogInterface::Initialize_Static -- global initialization function for the logging system.  Only ever invoke once
		per process.

		service_name -- the name of the executing process
		log_level -- logging level that the system should start at
		
**********************************************************************************************************************/
void CLogInterface::Initialize_Static( const std::wstring &service_name, ELogLevel log_level )
{
	FATAL_ASSERT( StaticInitialized == false );

	LogLock = NPlatform::Create_Simple_Mutex();

	ServiceName = service_name;
	LogLevel = log_level;

	// Create the logging directory if it does not exist
	if ( !NPlatform::Directory_Exists( LogSubdirectory ) )
	{
		NPlatform::Create_Directory( LogSubdirectory );
	}

	// Create the log archive directory if it does not exist
	if ( !NPlatform::Directory_Exists( ArchiveSubdirectory ) )
	{
		NPlatform::Create_Directory( ArchiveSubdirectory );
	}

	StaticInitialized = true;
}

/**********************************************************************************************************************
	CLogInterface::Shutdown_Static -- global shutdown function for the logging system.  Only ever invoke once
		per process.
		
**********************************************************************************************************************/
void CLogInterface::Shutdown_Static( void )
{
	if ( !StaticInitialized )
	{
		return;
	}

	Shutdown_Dynamic();

	delete LogLock;
	LogLock = nullptr;

	StaticInitialized = false;
}

/**********************************************************************************************************************
	CLogInterface::Initialize_Dynamic -- second level initialization function; ok to call multiple times as long as an
		intervening Shutdown_Dynamic is used.  

		delete_all_logs -- should we attempt to delete all log files corresponding to this process
		
**********************************************************************************************************************/
void CLogInterface::Initialize_Dynamic( bool delete_all_logs )
{
	static const uint64 SECONDS_PER_HOUR = 3600;

	FATAL_ASSERT( StaticInitialized == true );

	CSimplePlatformMutexLocker lock( LogLock );

	FATAL_ASSERT( DynamicInitialized == false );

	// Remove old logs if they exist, logs that are not removable are attached to currently running processes
	std::basic_ostringstream< wchar_t > file_pattern_string;
	file_pattern_string << LogPath << ServiceName << L"*.txt";

	std::vector< std::wstring > matching_file_names;
	NPlatform::Enumerate_Matching_Files( file_pattern_string.rdbuf()->str(), matching_file_names );

	uint64 current_time = CPlatformTime::Get_Raw_Time();

	// Try to remove all matching log files that are older than an hour
	for ( uint32 i = 0; i < matching_file_names.size(); ++i )
	{
		uint64 file_time = CPlatformTime::Get_File_Write_Raw_Time( LogPath + matching_file_names[ i ] );

		if ( delete_all_logs || CPlatformTime::Is_Raw_Time_Less_Than_Seconds( file_time, current_time, SECONDS_PER_HOUR ) )
		{
			NPlatform::Delete_File( LogPath + matching_file_names[ i ] );
		}
	}

	LogThread.reset( new CLoggingThreadTask );

	DynamicInitialized = true;
}

/**********************************************************************************************************************
	CLogInterface::Shutdown_Dynamic -- second level shutdown function  
		
**********************************************************************************************************************/
void CLogInterface::Shutdown_Dynamic( void )
{
	FATAL_ASSERT( StaticInitialized == true );

	if ( DynamicInitialized )
	{
		CSimplePlatformMutexLocker lock( LogLock );

		LogThread = nullptr;

		DynamicInitialized = false;
	}
}

/**********************************************************************************************************************
	CLogInterface::Service_Logging -- invoke the logging thread to actually perform logging
	
		current_time -- current time in seconds
		context -- execution context that the log thread should use
		
**********************************************************************************************************************/
void CLogInterface::Service_Logging( double current_time, const CThreadTaskExecutionContext &context )
{
	CSimplePlatformMutexLocker lock( LogLock );

	if ( LogThread.get() != nullptr )
	{
		IThreadTask *old_task = CThreadStatics::Get_Current_Thread_Task();

		CThreadStatics::Set_Current_Thread_Task( LogThread.get() );
		LogThread->Service( current_time, context );
		CThreadStatics::Set_Current_Thread_Task( old_task );
		LogThread->Flush_Partitioned_Messages();
	}
}

/**********************************************************************************************************************
	CLogInterface::Log -- primary logging function; forwards text to the log thread to be logged
	
		message -- string to be logged
		
**********************************************************************************************************************/
void CLogInterface::Log( const std::wstring &message )
{
	IThreadTask *thread_task = CThreadStatics::Get_Current_Thread_Task();
	if ( thread_task != nullptr )
	{
		thread_task->Log( message );
		return;
	}

	CConcurrencyManager *manager = CThreadStatics::Get_Concurrency_Manager();
	if ( manager != nullptr )
	{
		manager->Log( message );
	}
}

/**********************************************************************************************************************
	CLogInterface::Log -- logging function; forwards text to the log thread to be logged
	
		message -- string to be logged
		
**********************************************************************************************************************/
void CLogInterface::Log( const wchar_t *message )
{
	Log( std::wstring( message ) );
}

/**********************************************************************************************************************
	CLogInterface::Log -- logging function; forwards text to the log thread to be logged
	
		message_stream -- stream containing the string to be logged
		
**********************************************************************************************************************/
void CLogInterface::Log( const std::basic_ostringstream< wchar_t > &message_stream )
{
	Log( message_stream.rdbuf()->str() );
}


