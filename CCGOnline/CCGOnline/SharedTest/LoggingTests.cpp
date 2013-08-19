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
#include "Logging/LoggingProcess.h"
#include "Concurrency/ProcessMailbox.h"
#include "Concurrency/ProcessConstants.h"
#include "Concurrency/MailboxInterfaces.h"
#include "Concurrency/ProcessMessageFrame.h"
#include "Concurrency/Messaging/ExchangeMailboxMessages.h"
#include "Concurrency/Messaging/LoggingMessages.h"
#include "Concurrency/Messaging/ProcessManagementMessages.h"
#include "Concurrency/ProcessExecutionContext.h"
#include "Concurrency/ProcessStatics.h"
#include "tbb/task.h"
#include "Time/TimeType.h"
#include "PlatformFileSystem.h"
#include "Concurrency/ProcessID.h"

class CLoggingVirtualProcessTester
{
	public:

		CLoggingVirtualProcessTester( void ) :
			LoggingProcess( nullptr ),
			LoggingMailbox( nullptr )
		{
			LoggingProcess = static_pointer_cast< CLoggingProcess >( CLogInterface::Get_Logging_Process() );
		}

		~CLoggingVirtualProcessTester()
		{
			LoggingProcess = nullptr;
		}

		void Initialize( void )
		{
			LoggingProcess->Initialize( EProcessID::LOGGING );
			LoggingMailbox.reset( new CProcessMailbox( EProcessID::LOGGING, LOGGING_PROCESS_PROPERTIES ) );
			LoggingProcess->Set_My_Mailbox( LoggingMailbox->Get_Readable_Mailbox() );
		}

		void Service( void )
		{
			tbb::task *task = reinterpret_cast< tbb::task * >( 1 );
			CProcessExecutionContext context( task, 0.0 );
			CLogInterface::Service_Logging( context );
		}

		shared_ptr< CWriteOnlyMailbox > Get_Writable_Mailbox( void ) const { return LoggingMailbox->Get_Writable_Mailbox(); }

		shared_ptr< CLoggingProcess > Get_Logging_Virtual_Process( void ) const { return LoggingProcess; }

	private:

		shared_ptr< CLoggingProcess > LoggingProcess;

		shared_ptr< CProcessMailbox > LoggingMailbox;
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

static const EProcessID::Enum TEST_KEY1 = static_cast< EProcessID::Enum >( EProcessID::FIRST_FREE_ID );
static const EProcessID::Enum TEST_KEY2 = static_cast< EProcessID::Enum >( EProcessID::FIRST_FREE_ID + 1 );

static const SProcessProperties TEST_PROPS1( EProcessSubject::NEXT_FREE_VALUE, 1, 1, 1 );
static const SProcessProperties TEST_PROPS2( EProcessSubject::NEXT_FREE_VALUE + 1, 1, 1, 1 );

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

	unique_ptr< CProcessMessageFrame > ai_frame( new CProcessMessageFrame( TEST_KEY1 ) );
	ai_frame->Add_Message( unique_ptr< const IProcessMessage >( new CLogRequestMessage( TEST_PROPS1, LOG_TEST_MESSAGE ) ) );
	ai_frame->Add_Message( unique_ptr< const IProcessMessage >( new CLogRequestMessage( TEST_PROPS1, LOG_TEST_MESSAGE ) ) );
	log_tester.Get_Writable_Mailbox()->Add_Frame( ai_frame );

	unique_ptr< CProcessMessageFrame > manager_frame( new CProcessMessageFrame( EProcessID::CONCURRENCY_MANAGER ) );
	manager_frame->Add_Message( unique_ptr< const IProcessMessage >( new CLogRequestMessage( MANAGER_PROCESS_PROPERTIES, LOG_TEST_MESSAGE ) ) );
	log_tester.Get_Writable_Mailbox()->Add_Frame( manager_frame );

	unique_ptr< CProcessMessageFrame > db_frame( new CProcessMessageFrame( TEST_KEY2 ) );
	db_frame->Add_Message( unique_ptr< const IProcessMessage >( new CLogRequestMessage( TEST_PROPS2, LOG_TEST_MESSAGE ) ) );
	log_tester.Get_Writable_Mailbox()->Add_Frame( db_frame );

	unique_ptr< CProcessMessageFrame > shutdown_frame( new CProcessMessageFrame( EProcessID::CONCURRENCY_MANAGER ) );
	shutdown_frame->Add_Message( unique_ptr< const IProcessMessage >( new CShutdownSelfRequest( false ) ) );
	log_tester.Get_Writable_Mailbox()->Add_Frame( shutdown_frame );

	log_tester.Service();

	NPlatform::Enumerate_Matching_Files( LOG_FILE_PATTERN, file_names );

	ASSERT_TRUE( file_names.size() == 3 );
	for ( uint32 i = 0; i < file_names.size(); i++ )
	{
		Verify_Log_File( file_names[ i ] );
	}
}

class CDummyProcess : public CTaskProcessBase
{
	public:
		
		typedef CTaskProcessBase BASECLASS;

		CDummyProcess( const SProcessProperties &properties ) :
			BASECLASS( properties )
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

	shared_ptr< CDummyProcess > dummy_process( new CDummyProcess( TEST_PROPS1 ) );
	dummy_process->Initialize( EProcessID::FIRST_FREE_ID );

	shared_ptr< CProcessMailbox > dummy_mailbox( new CProcessMailbox( EProcessID::FIRST_FREE_ID, TEST_PROPS1 ) );
	dummy_process->Set_My_Mailbox( dummy_mailbox->Get_Readable_Mailbox() );
	dummy_process->Set_Logging_Mailbox( log_tester.Get_Writable_Mailbox() );

	CProcessStatics::Set_Current_Process( dummy_process.get() );

	CProcessExecutionContext context( nullptr, 0.0 );
	dummy_process->Run( context );

	WLOG( LL_HIGH, L"This is another test, but I have to end with " << LOG_TEST_MESSAGE );
	WLOG( LL_HIGH, L"test: " << 5 << LOG_TEST_MESSAGE );

	CProcessStatics::Set_Current_Process( nullptr );

	dummy_process->Flush_System_Messages();

	unique_ptr< CProcessMessageFrame > shutdown_frame( new CProcessMessageFrame( EProcessID::CONCURRENCY_MANAGER ) );
	shutdown_frame->Add_Message( unique_ptr< const IProcessMessage >( new CShutdownSelfRequest( false ) ) );
	log_tester.Get_Writable_Mailbox()->Add_Frame( shutdown_frame );

	log_tester.Service();

	NPlatform::Enumerate_Matching_Files( LOG_FILE_PATTERN, file_names );
	ASSERT_TRUE( file_names.size() == 1 );

	for ( uint32 i = 0; i < file_names.size(); i++ )
	{
		Verify_Log_File( file_names[ i ] );
	}
}