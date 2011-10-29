/**********************************************************************************************************************

	[Placeholder for eventual source license]

	LoggingTests.cpp
		defines unit tests for logging functionality

	(c) Copyright 2011, Bret Ambrose.  All rights reserved.

**********************************************************************************************************************/

#include "stdafx.h"

#include <fstream>
#include <iostream>

#include "Logging/LogInterface.h"
#include "Logging/LoggingThread.h"
#include "Concurrency/ThreadConnection.h"
#include "Concurrency/ThreadConstants.h"
#include "Concurrency/ThreadInterfaces.h"
#include "Concurrency/ThreadMessageFrame.h"
#include "Concurrency/ThreadMessages/ExchangeInterfaceMessages.h"
#include "Concurrency/ThreadMessages/LoggingMessages.h"
#include "Concurrency/ThreadMessages/ThreadManagementMessages.h"
#include "Concurrency/ThreadSubject.h"
#include "Concurrency/ThreadTaskExecutionContext.h"
#include "Concurrency/ThreadStatics.h"
#include "tbb/task.h"
#include "Time/TimeType.h"
#include "PlatformFileSystem.h"

class CLoggingThreadTester
{
	public:

		CLoggingThreadTester( void ) :
			LoggingThread( nullptr ),
			LoggingConnection( nullptr )
		{
			LoggingThread = static_pointer_cast< CLoggingThreadTask >( CLogInterface::Get_Logging_Thread() );
		}

		~CLoggingThreadTester()
		{
			LoggingThread = nullptr;
		}

		void Initialize( void )
		{
			LoggingThread->Initialize();
			LoggingConnection.reset( new CThreadConnection( LOG_THREAD_KEY ) );
			LoggingThread->Set_Read_Interface( LoggingConnection->Get_Reader_Interface() );
		}

		void Service( void )
		{
			tbb::task *task = reinterpret_cast< tbb::task * >( 1 );
			CThreadTaskExecutionContext context( task );
			CLogInterface::Service_Logging( 0.0, context );
		}

		shared_ptr< CWriteOnlyThreadInterface > Get_Write_Interface( void ) const { return LoggingConnection->Get_Writer_Interface(); }

		shared_ptr< CLoggingThreadTask > Get_Logging_Thread( void ) const { return LoggingThread; }

	private:

		shared_ptr< CLoggingThreadTask > LoggingThread;

		shared_ptr< CThreadConnection > LoggingConnection;
};

static const std::wstring LOG_FILE_PATTERN( L"Logs\\*.txt" );

class LoggingTests : public testing::Test 
{
	protected:  

		virtual void SetUp( void )
		{
			CLogInterface::Initialize_Dynamic( true );
		}

		virtual void TearDown( void )
		{
			CLogInterface::Shutdown_Dynamic();
		}

	private:

};

TEST_F( LoggingTests, Log_Level )
{
	CLogInterface::Set_Log_Level( LL_MEDIUM );
	ASSERT_TRUE( CLogInterface::Get_Log_Level() == LL_MEDIUM );
}

static const SThreadKey AI_KEY( TS_AI, 1, 1 );
static const SThreadKey DB_KEY( TS_DATABASE, 1, 1 );
static const std::wstring LOG_TEST_MESSAGE( L"Testing" );

void Verify_Log_File( const std::wstring &file_name )
{
	std::wstring full_name = std::wstring( L"Logs\\" ) + file_name;
	std::basic_ifstream< wchar_t > file( full_name.c_str(), std::ios_base::in );

	ASSERT_TRUE( file.is_open() );
	ASSERT_TRUE( file.good() );

	bool got_a_non_empty_line = false;
	while( file.good() )
	{
		std::wstring line;
		std::getline( file, line );
		if ( line.size() > 0 )
		{
			got_a_non_empty_line = true;
			ASSERT_TRUE( line.size() > LOG_TEST_MESSAGE.size() );

			std::wstring line_end = line.substr( line.size() - LOG_TEST_MESSAGE.size(), LOG_TEST_MESSAGE.size() );
			ASSERT_TRUE( line_end == LOG_TEST_MESSAGE );
		}
	}

	ASSERT_TRUE( got_a_non_empty_line );

	file.close();
}

TEST_F( LoggingTests, Direct_Logging )
{
	std::vector< std::wstring > file_names;
	NPlatform::Enumerate_Matching_Files( LOG_FILE_PATTERN, file_names );

	ASSERT_TRUE( file_names.size() == 0 );

	CLoggingThreadTester log_tester;
	log_tester.Initialize();

	shared_ptr< CThreadMessageFrame > ai_frame( new CThreadMessageFrame( AI_KEY ) );
	ai_frame->Add_Message( shared_ptr< const IThreadMessage >( new CLogRequestMessage( AI_KEY, LOG_TEST_MESSAGE ) ) );
	ai_frame->Add_Message( shared_ptr< const IThreadMessage >( new CLogRequestMessage( AI_KEY, LOG_TEST_MESSAGE ) ) );
	log_tester.Get_Write_Interface()->Add_Frame( ai_frame );

	shared_ptr< CThreadMessageFrame > manager_frame( new CThreadMessageFrame( MANAGER_THREAD_KEY ) );
	manager_frame->Add_Message( shared_ptr< const IThreadMessage >( new CLogRequestMessage( MANAGER_THREAD_KEY, LOG_TEST_MESSAGE ) ) );
	log_tester.Get_Write_Interface()->Add_Frame( manager_frame );

	shared_ptr< CThreadMessageFrame > db_frame( new CThreadMessageFrame( DB_KEY ) );
	db_frame->Add_Message( shared_ptr< const IThreadMessage >( new CLogRequestMessage( DB_KEY, LOG_TEST_MESSAGE ) ) );
	log_tester.Get_Write_Interface()->Add_Frame( db_frame );

	shared_ptr< CThreadMessageFrame > shutdown_frame( new CThreadMessageFrame( MANAGER_THREAD_KEY ) );
	shutdown_frame->Add_Message( shared_ptr< const IThreadMessage >( new CShutdownThreadRequest( false ) ) );
	log_tester.Get_Write_Interface()->Add_Frame( shutdown_frame );

	log_tester.Service();

	NPlatform::Enumerate_Matching_Files( LOG_FILE_PATTERN, file_names );

	ASSERT_TRUE( file_names.size() == 3 );
	for ( uint32 i = 0; i < file_names.size(); i++ )
	{
		Verify_Log_File( file_names[ i ] );
	}
}

class CDummyThread : public CThreadTaskBase
{
	public:
		
		typedef CThreadTaskBase BASECLASS;

		CDummyThread( const SThreadKey &key ) :
			BASECLASS( key )
		{}

		virtual ETimeType Get_Time_Type( void ) const { return TT_GAME_TIME; }
		virtual bool Is_Root_Thread( void ) const { return true; }

};

TEST_F( LoggingTests, Static_Logging )
{
	std::vector< std::wstring > file_names;
	NPlatform::Enumerate_Matching_Files( LOG_FILE_PATTERN, file_names );

	ASSERT_TRUE( file_names.size() == 0 );

	CLogInterface::Set_Log_Level( LL_HIGH );

	CLoggingThreadTester log_tester;
	log_tester.Initialize();

	shared_ptr< CDummyThread > dummy_thread( new CDummyThread( AI_KEY ) );
	dummy_thread->Initialize();

	shared_ptr< CThreadConnection > dummy_connection( new CThreadConnection( AI_KEY ) );
	dummy_thread->Set_Read_Interface( dummy_connection->Get_Reader_Interface() );

	shared_ptr< CThreadMessageFrame > manager_frame( new CThreadMessageFrame( MANAGER_THREAD_KEY ) );
	manager_frame->Add_Message( shared_ptr< const IThreadMessage >( new CAddInterfaceMessage( LOG_THREAD_KEY, log_tester.Get_Write_Interface() ) ) );

	dummy_connection->Get_Writer_Interface()->Add_Frame( manager_frame );

	CThreadStatics::Set_Current_Thread_Task( dummy_thread.get() );

	CThreadTaskExecutionContext context( nullptr );
	dummy_thread->Service( 0.0, context );

	LOG( LL_HIGH, L"This is another test, but I have to end with " << LOG_TEST_MESSAGE );
	LOG( LL_HIGH, L"test: " << 5 << LOG_TEST_MESSAGE );

	CThreadStatics::Set_Current_Thread_Task( nullptr );

	dummy_thread->Flush_Partitioned_Messages();

	shared_ptr< CThreadMessageFrame > shutdown_frame( new CThreadMessageFrame( MANAGER_THREAD_KEY ) );
	shutdown_frame->Add_Message( shared_ptr< const IThreadMessage >( new CShutdownThreadRequest( false ) ) );
	log_tester.Get_Write_Interface()->Add_Frame( shutdown_frame );

	log_tester.Service();

	NPlatform::Enumerate_Matching_Files( LOG_FILE_PATTERN, file_names );
	ASSERT_TRUE( file_names.size() == 1 );

	for ( uint32 i = 0; i < file_names.size(); i++ )
	{
		Verify_Log_File( file_names[ i ] );
	}
}