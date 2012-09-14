/**********************************************************************************************************************

	LoggingTests.cpp
		defines unit tests for logging functionality

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

#include <fstream>
#include <iostream>

#include "Logging/LogInterface.h"
#include "Logging/LoggingVirtualProcess.h"
#include "Concurrency/VirtualProcessMailbox.h"
#include "Concurrency/VirtualProcessConstants.h"
#include "Concurrency/MailboxInterfaces.h"
#include "Concurrency/VirtualProcessMessageFrame.h"
#include "Concurrency/Messaging/ExchangeMailboxMessages.h"
#include "Concurrency/Messaging/LoggingMessages.h"
#include "Concurrency/Messaging/VirtualProcessManagementMessages.h"
#include "Concurrency/ThreadSubject.h"
#include "Concurrency/VirtualProcessExecutionContext.h"
#include "Concurrency/VirtualProcessStatics.h"
#include "tbb/task.h"
#include "Time/TimeType.h"
#include "PlatformFileSystem.h"
#include "Concurrency/VirtualProcessID.h"

class CLoggingVirtualProcessTester
{
	public:

		CLoggingVirtualProcessTester( void ) :
			LoggingVirtualProcess( nullptr ),
			LoggingMailbox( nullptr )
		{
			LoggingVirtualProcess = static_pointer_cast< CLoggingVirtualProcess >( CLogInterface::Get_Logging_Process() );
		}

		~CLoggingVirtualProcessTester()
		{
			LoggingVirtualProcess = nullptr;
		}

		void Initialize( void )
		{
			LoggingVirtualProcess->Initialize( EVirtualProcessID::LOGGING );
			LoggingMailbox.reset( new CVirtualProcessMailbox( LOG_THREAD_KEY ) );
			LoggingVirtualProcess->Set_My_Mailbox( LoggingMailbox->Get_Readable_Mailbox() );
		}

		void Service( void )
		{
			tbb::task *task = reinterpret_cast< tbb::task * >( 1 );
			CVirtualProcessExecutionContext context( task );
			CLogInterface::Service_Logging( 0.0, context );
		}

		shared_ptr< CWriteOnlyMailbox > Get_Writable_Mailbox( void ) const { return LoggingMailbox->Get_Writable_Mailbox(); }

		shared_ptr< CLoggingVirtualProcess > Get_Logging_Virtual_Process( void ) const { return LoggingVirtualProcess; }

	private:

		shared_ptr< CLoggingVirtualProcess > LoggingVirtualProcess;

		shared_ptr< CVirtualProcessMailbox > LoggingMailbox;
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

	CLoggingVirtualProcessTester log_tester;
	log_tester.Initialize();

	shared_ptr< CVirtualProcessMessageFrame > ai_frame( new CVirtualProcessMessageFrame( AI_KEY ) );
	ai_frame->Add_Message( shared_ptr< const IVirtualProcessMessage >( new CLogRequestMessage( AI_KEY, LOG_TEST_MESSAGE ) ) );
	ai_frame->Add_Message( shared_ptr< const IVirtualProcessMessage >( new CLogRequestMessage( AI_KEY, LOG_TEST_MESSAGE ) ) );
	log_tester.Get_Writable_Mailbox()->Add_Frame( ai_frame );

	shared_ptr< CVirtualProcessMessageFrame > manager_frame( new CVirtualProcessMessageFrame( MANAGER_THREAD_KEY ) );
	manager_frame->Add_Message( shared_ptr< const IVirtualProcessMessage >( new CLogRequestMessage( MANAGER_THREAD_KEY, LOG_TEST_MESSAGE ) ) );
	log_tester.Get_Writable_Mailbox()->Add_Frame( manager_frame );

	shared_ptr< CVirtualProcessMessageFrame > db_frame( new CVirtualProcessMessageFrame( DB_KEY ) );
	db_frame->Add_Message( shared_ptr< const IVirtualProcessMessage >( new CLogRequestMessage( DB_KEY, LOG_TEST_MESSAGE ) ) );
	log_tester.Get_Writable_Mailbox()->Add_Frame( db_frame );

	shared_ptr< CVirtualProcessMessageFrame > shutdown_frame( new CVirtualProcessMessageFrame( MANAGER_THREAD_KEY ) );
	shutdown_frame->Add_Message( shared_ptr< const IVirtualProcessMessage >( new CShutdownSelfRequest( false ) ) );
	log_tester.Get_Writable_Mailbox()->Add_Frame( shutdown_frame );

	log_tester.Service();

	NPlatform::Enumerate_Matching_Files( LOG_FILE_PATTERN, file_names );

	ASSERT_TRUE( file_names.size() == 3 );
	for ( uint32 i = 0; i < file_names.size(); i++ )
	{
		Verify_Log_File( file_names[ i ] );
	}
}

class CDummyProcess : public CVirtualProcessBase
{
	public:
		
		typedef CVirtualProcessBase BASECLASS;

		CDummyProcess( const SThreadKey &key ) :
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

	CLoggingVirtualProcessTester log_tester;
	log_tester.Initialize();

	shared_ptr< CDummyProcess > dummy_process( new CDummyProcess( AI_KEY ) );
	dummy_process->Initialize( EVirtualProcessID::FIRST_FREE_ID );

	shared_ptr< CVirtualProcessMailbox > dummy_mailbox( new CVirtualProcessMailbox( AI_KEY ) );
	dummy_process->Set_My_Mailbox( dummy_mailbox->Get_Readable_Mailbox() );

	shared_ptr< CVirtualProcessMessageFrame > manager_frame( new CVirtualProcessMessageFrame( MANAGER_THREAD_KEY ) );
	manager_frame->Add_Message( shared_ptr< const IVirtualProcessMessage >( new CAddMailboxMessage( LOG_THREAD_KEY, log_tester.Get_Writable_Mailbox() ) ) );

	dummy_mailbox->Get_Writable_Mailbox()->Add_Frame( manager_frame );

	CVirtualProcessStatics::Set_Current_Virtual_Process( dummy_process.get() );

	CVirtualProcessExecutionContext context( nullptr );
	dummy_process->Service( 0.0, context );

	LOG( LL_HIGH, L"This is another test, but I have to end with " << LOG_TEST_MESSAGE );
	LOG( LL_HIGH, L"test: " << 5 << LOG_TEST_MESSAGE );

	CVirtualProcessStatics::Set_Current_Virtual_Process( nullptr );

	dummy_process->Flush_System_Messages();

	shared_ptr< CVirtualProcessMessageFrame > shutdown_frame( new CVirtualProcessMessageFrame( MANAGER_THREAD_KEY ) );
	shutdown_frame->Add_Message( shared_ptr< const IVirtualProcessMessage >( new CShutdownSelfRequest( false ) ) );
	log_tester.Get_Writable_Mailbox()->Add_Frame( shutdown_frame );

	log_tester.Service();

	NPlatform::Enumerate_Matching_Files( LOG_FILE_PATTERN, file_names );
	ASSERT_TRUE( file_names.size() == 1 );

	for ( uint32 i = 0; i < file_names.size(); i++ )
	{
		Verify_Log_File( file_names[ i ] );
	}
}