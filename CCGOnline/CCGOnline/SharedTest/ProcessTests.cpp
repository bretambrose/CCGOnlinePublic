/**********************************************************************************************************************

	ProcessTests.cpp
		defines unit tests for process related functionality

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

#include "Concurrency/TaskProcessBase.h"
#include "Concurrency/ThreadProcessBase.h"
#include "Concurrency/ProcessSubject.h"
#include "Concurrency/ProcessMailbox.h"
#include "Concurrency/ProcessConstants.h"
#include "Concurrency/ProcessStatics.h"
#include "Concurrency/ProcessExecutionContext.h"
#include "Concurrency/MailboxInterfaces.h"
#include "Concurrency/ProcessMessageFrame.h"
#include "Concurrency/Messaging/ProcessManagementMessages.h"
#include "Concurrency/Messaging/ExchangeMailboxMessages.h"
#include "Concurrency/Messaging/LoggingMessages.h"
#include "Concurrency/ProcessID.h"
#include "Time/TimeType.h"
#include "TaskScheduler/ScheduledTask.h"
#include "TaskScheduler/TaskScheduler.h"
#include "Logging/LogInterface.h"
#include "Helpers/ProcessHelpers.h"
#include "PlatformProcess.h"
#include "PlatformThread.h"

class ProcessTests : public testing::Test 
{
	protected:  


};

class CTestProcessTask : public CTaskProcessBase
{
	public:

		typedef CTaskProcessBase BASECLASS;

		CTestProcessTask( const SProcessProperties &properties ) :
			BASECLASS( properties )
		{}

		virtual ETimeType Get_Time_Type( void ) const { return TT_GAME_TIME; }
		virtual bool Is_Root_Thread( void ) const { return true; }

	protected:

	private:
};

class CTestThreadProcess : public CThreadProcessBase
{
	public:

		typedef CThreadProcessBase BASECLASS;

		CTestThreadProcess( const SProcessProperties &properties ) :
			BASECLASS( properties ),
			FramesCompleted( 0 )
		{}

		virtual ETimeType Get_Time_Type( void ) const { return TT_GAME_TIME; }
		virtual bool Is_Root_Thread( void ) const { return true; }

		uint64 Get_Frames_Completed( void ) const { return FramesCompleted; }

	protected:

		virtual void Per_Frame_Logic_End( void ) { ++FramesCompleted; }

		virtual uint32 Get_Sleep_Interval_In_Milliseconds( void ) const { return 10; }

	private:

		uint64 FramesCompleted;
};


class CBasicServiceTestTask : public CScheduledTask
{
	public:

		typedef CScheduledTask BASECLASS;

		CBasicServiceTestTask( double execute_time_seconds ) :
			BASECLASS( execute_time_seconds )
		{}

		virtual bool Execute( double /*time_seconds*/, double & /*reschedule_time_seconds*/ )
		{
			CTaskProcessBaseTester::Set_Has_Process_Service_Executed();

			return false;
		}
};

namespace ETestProcessSubject
{
	enum Enum
	{
		AI = EProcessSubject::NEXT_FREE_VALUE,
		DATABASE,
		UI
	};
}

SProcessProperties AI_PROPS( ETestProcessSubject::AI  );


TEST_F( ProcessTests, Basic_Service_And_Reschedule )
{
	static const double FIRST_SERVICE_TIME = 1.0;
	static const double SECOND_SERVICE_TIME = 4.99;
	static const double THIRD_SERVICE_TIME = 5.0;

	CTaskProcessBaseTester process_tester( new CTestProcessTask( AI_PROPS ) );

	ASSERT_FALSE( CTaskProcessBaseTester::Get_Has_Process_Service_Executed() );

	shared_ptr< CScheduledTask > simple_task( new CBasicServiceTestTask( 5.0 ) );
	process_tester.Get_Process()->Get_Task_Scheduler()->Submit_Task( simple_task );

	process_tester.Service( FIRST_SERVICE_TIME );
	ASSERT_FALSE( CTaskProcessBaseTester::Get_Has_Process_Service_Executed() );

	std::vector< shared_ptr< CProcessMessageFrame > > frames;
	process_tester.Get_Manager_Proxy()->Get_Readable_Mailbox()->Remove_Frames( frames );
	ASSERT_TRUE( frames.size() == 1 );

	shared_ptr< CProcessMessageFrame > frame = frames[ 0 ];
	for ( auto iter = frame->Get_Frame_Begin(); iter != frame->Get_Frame_End(); ++iter )
	{
		const IProcessMessage *raw_message = iter->get();

		ASSERT_TRUE( Loki::TypeInfo( typeid( *raw_message ) ) == Loki::TypeInfo( typeid( CRescheduleProcessMessage ) ) );

		const CRescheduleProcessMessage *reschedule_message = static_cast< const CRescheduleProcessMessage * >( raw_message );

		ASSERT_DOUBLE_EQ( reschedule_message->Get_Reschedule_Time(), FIRST_SERVICE_TIME + process_tester.Get_Reschedule_Interval() );
	}

	frames.clear();

	process_tester.Service( SECOND_SERVICE_TIME );
	ASSERT_FALSE( CTaskProcessBaseTester::Get_Has_Process_Service_Executed() );

	process_tester.Get_Manager_Proxy()->Get_Readable_Mailbox()->Remove_Frames( frames );
	ASSERT_TRUE( frames.size() == 1 );

	frame = frames[ 0 ];
	for ( auto iter = frame->Get_Frame_Begin(); iter != frame->Get_Frame_End(); ++iter )
	{
		const IProcessMessage *raw_message = iter->get();

		ASSERT_TRUE( Loki::TypeInfo( typeid( *raw_message ) ) == Loki::TypeInfo( typeid( CRescheduleProcessMessage ) ) );

		const CRescheduleProcessMessage *reschedule_message = static_cast< const CRescheduleProcessMessage * >( raw_message );

		ASSERT_DOUBLE_EQ( reschedule_message->Get_Reschedule_Time(), THIRD_SERVICE_TIME );
	}

	process_tester.Service( THIRD_SERVICE_TIME );
	ASSERT_TRUE( CTaskProcessBaseTester::Get_Has_Process_Service_Executed() );
}

static const SProcessProperties DB_PROPS( ETestProcessSubject::DATABASE );
static const EProcessID::Enum DB_PROCESS_ID = static_cast< EProcessID::Enum >( EProcessID::FIRST_FREE_ID + 1 );

class CSendAddMailboxMessageServiceTask : public CScheduledTask
{
	public:

		typedef CScheduledTask BASECLASS;

		CSendAddMailboxMessageServiceTask( double execute_time_seconds, const shared_ptr< CWriteOnlyMailbox > &mailbox ) :
			BASECLASS( execute_time_seconds ),
			Mailbox( mailbox )
		{}

		virtual bool Execute( double time_seconds, double &reschedule_time_seconds )
		{
			CProcessStatics::Get_Current_Process()->Send_Process_Message( DB_PROCESS_ID, shared_ptr< const IProcessMessage >( new CAddMailboxMessage( Mailbox ) ) );

			reschedule_time_seconds = time_seconds + .1;
			return true;
		}

	private:

		shared_ptr< CWriteOnlyMailbox > Mailbox;
};

TEST_F( ProcessTests, Send_Message )
{
	CTaskProcessBaseTester process_tester( new CTestProcessTask( AI_PROPS ) );

	shared_ptr< CScheduledTask > simple_task( new CSendAddMailboxMessageServiceTask( 0.0, process_tester.Get_Self_Proxy()->Get_Writable_Mailbox() ) );
	process_tester.Get_Process()->Get_Task_Scheduler()->Submit_Task( simple_task );

	process_tester.Service( 1.0 );

	// Verify pending message due to no interface
	auto frame_table = process_tester.Get_Frame_Table();
	ASSERT_TRUE( frame_table.find( DB_PROCESS_ID ) != frame_table.end() );
	shared_ptr< CProcessMessageFrame > db_frame = frame_table.find( DB_PROCESS_ID )->second;
	for ( auto iter = db_frame->Get_Frame_Begin(); iter != db_frame->Get_Frame_End(); ++iter )
	{
		const IProcessMessage *raw_message = iter->get();

		ASSERT_TRUE( Loki::TypeInfo( typeid( *raw_message ) ) == Loki::TypeInfo( typeid( CAddMailboxMessage ) ) );

		const CAddMailboxMessage *add_mailbox_message = static_cast< const CAddMailboxMessage * >( raw_message );

		ASSERT_TRUE( add_mailbox_message->Get_Mailbox().get() == process_tester.Get_Self_Proxy()->Get_Writable_Mailbox().get() );
	}

	shared_ptr< CProcessMailbox > db_conn( new CProcessMailbox( DB_PROCESS_ID, DB_PROPS ) );

	// notify the thread of the db interface
	shared_ptr< CProcessMessageFrame > added_frame( new CProcessMessageFrame( EProcessID::CONCURRENCY_MANAGER ) );
	added_frame->Add_Message( shared_ptr< const IProcessMessage >( new CAddMailboxMessage( db_conn->Get_Writable_Mailbox() ) ) );
	process_tester.Get_Self_Proxy()->Get_Writable_Mailbox()->Add_Frame( added_frame );

	process_tester.Service( 2.0 );

	// verify new interface added
	auto interfaces = process_tester.Get_Mailbox_Table();
	ASSERT_TRUE( interfaces.size() == 1 );	
	ASSERT_TRUE( interfaces.find( DB_PROCESS_ID ) != interfaces.end() );

	// verify both (send task is recurrent) messages sent to the db thread's mailbox
	frame_table = process_tester.Get_Frame_Table();
	ASSERT_TRUE( frame_table.size() == 0 );

	std::vector< shared_ptr< CProcessMessageFrame > > frames;
	db_conn->Get_Readable_Mailbox()->Remove_Frames( frames );

	ASSERT_TRUE( frames.size() == 1 );

	shared_ptr< CProcessMessageFrame > frame = frames[ 0 ];
	for ( auto iter = frame->Get_Frame_Begin(); iter != frame->Get_Frame_End(); ++iter )
	{
		const IProcessMessage *raw_message = iter->get();

		ASSERT_TRUE( Loki::TypeInfo( typeid( *raw_message ) ) == Loki::TypeInfo( typeid( CAddMailboxMessage ) ) );

		const CAddMailboxMessage *add_mailbox_message = static_cast< const CAddMailboxMessage * >( raw_message );

		ASSERT_TRUE( add_mailbox_message->Get_Mailbox().get() == process_tester.Get_Self_Proxy()->Get_Writable_Mailbox().get() );
	}
}

TEST_F( ProcessTests, Add_Mailbox_And_Logging )
{
	static const std::wstring LOG_MESSAGE_1( L"Log Test 1" );
	static const std::wstring LOG_MESSAGE_2( L"Log Test 2" );
	static const std::wstring LOG_MESSAGE_3( L"Log Test 3" );

	CTaskProcessBaseTester process_tester( new CTestProcessTask( AI_PROPS ) );

	// vanishes because no context to route to
	CLogInterface::Log( LOG_MESSAGE_1 );

	process_tester.Log( LOG_MESSAGE_2 );

	// Verify pending message due to no interface
	shared_ptr< CProcessMessageFrame > log_frame = process_tester.Get_Log_Frame();
	ASSERT_TRUE( log_frame.get() != nullptr );
	
	for ( auto iter = log_frame->Get_Frame_Begin(); iter != log_frame->Get_Frame_End(); ++iter )
	{
		const IProcessMessage *raw_message = iter->get();

		ASSERT_TRUE( Loki::TypeInfo( typeid( *raw_message ) ) == Loki::TypeInfo( typeid( CLogRequestMessage ) ) );

		const CLogRequestMessage *log_message = static_cast< const CLogRequestMessage * >( raw_message );

		ASSERT_TRUE( log_message->Get_Source_Properties() == AI_PROPS );
		ASSERT_TRUE( log_message->Get_Message() == LOG_MESSAGE_2 );
	}
	
	shared_ptr< CProcessMailbox > log_conn( new CProcessMailbox( LOGGING_PROCESS_ID, LOGGING_PROCESS_PROPERTIES ) );

	// notify the thread of the log interface
	process_tester.Set_Logging_Mailbox( log_conn->Get_Writable_Mailbox() );

	// verify new interface added
	ASSERT_TRUE( process_tester.Get_Mailbox_Table().size() == 0 );	// 
	ASSERT_TRUE( process_tester.Get_Logging_Mailbox().get() != nullptr );

	process_tester.Service( 2.0 );

	// verify second log message sent to the log thread's mailbox
	auto frame_table = process_tester.Get_Frame_Table();
	ASSERT_TRUE( frame_table.size() == 0 );

	std::vector< shared_ptr< CProcessMessageFrame > > frames;
	log_conn->Get_Readable_Mailbox()->Remove_Frames( frames );

	ASSERT_TRUE( frames.size() == 1 );

	shared_ptr< CProcessMessageFrame > frame = frames[ 0 ];
	ASSERT_TRUE( frame->Get_Process_ID() == AI_PROCESS_ID );
	for ( auto iter = frame->Get_Frame_Begin(); iter != frame->Get_Frame_End(); ++iter )
	{
		const IProcessMessage *raw_message = iter->get();

		ASSERT_TRUE( Loki::TypeInfo( typeid( *raw_message ) ) == Loki::TypeInfo( typeid( CLogRequestMessage ) ) );

		const CLogRequestMessage *log_message = static_cast< const CLogRequestMessage * >( raw_message );

		ASSERT_TRUE( log_message->Get_Source_Properties() == AI_PROPS );
		ASSERT_TRUE( log_message->Get_Message() == LOG_MESSAGE_2 );
	}

	process_tester.Log( LOG_MESSAGE_3 );

	process_tester.Service( 3.0 );

	// verify third log message sent to the log thread's mailbox
	frame_table = process_tester.Get_Frame_Table();
	ASSERT_TRUE( frame_table.size() == 0 );

	log_conn->Get_Readable_Mailbox()->Remove_Frames( frames );

	ASSERT_TRUE( frames.size() == 1 );

	frame = frames[ 0 ];
	for ( auto iter = frame->Get_Frame_Begin(); iter != frame->Get_Frame_End(); ++iter )
	{
		const IProcessMessage *raw_message = iter->get();

		ASSERT_TRUE( Loki::TypeInfo( typeid( *raw_message ) ) == Loki::TypeInfo( typeid( CLogRequestMessage ) ) );

		const CLogRequestMessage *log_message = static_cast< const CLogRequestMessage * >( raw_message );

		ASSERT_TRUE( log_message->Get_Source_Properties() == AI_PROPS );
		ASSERT_TRUE( log_message->Get_Message() == LOG_MESSAGE_3 );
	}

}

static const SProcessProperties UI_PROPS( ETestProcessSubject::UI );
static const EProcessID::Enum UI_PROCESS_ID = static_cast< EProcessID::Enum >( EProcessID::FIRST_FREE_ID + 2 );

class CSendMailboxMessageTask : public CScheduledTask
{
	public:

		typedef CScheduledTask BASECLASS;

		CSendMailboxMessageTask( double execute_time_seconds, const shared_ptr< CWriteOnlyMailbox > &mailbox, EProcessID::Enum target_process_id ) :
			BASECLASS( execute_time_seconds ),
			Mailbox( mailbox ),
			TargetProcessID( target_process_id )
		{}

		virtual bool Execute( double /*time_seconds*/, double & /*reschedule_time_seconds*/ )
		{
			CProcessStatics::Get_Current_Process()->Send_Process_Message( TargetProcessID, shared_ptr< const IProcessMessage >( new CAddMailboxMessage( Mailbox ) ) );
			return false;
		}

	private:

		shared_ptr< CWriteOnlyMailbox > Mailbox;
		EProcessID::Enum TargetProcessID;
};

TEST_F( ProcessTests, Shutdown_Interface )
{
	static const std::wstring LOG_MESSAGE( L"Log Test" );

	CTaskProcessBaseTester process_tester( new CTestProcessTask( AI_PROPS ) );
	EProcessID::Enum tester_id = process_tester.Get_Process()->Get_ID();

	shared_ptr< CProcessMailbox > log_conn( new CProcessMailbox( LOGGING_PROCESS_ID, LOGGING_PROCESS_PROPERTIES ) );
	shared_ptr< CProcessMailbox > ui_conn( new CProcessMailbox( UI_PROCESS_ID, UI_PROPS ) );

	shared_ptr< CProcessMessageFrame > added_frame( new CProcessMessageFrame( EProcessID::CONCURRENCY_MANAGER ) );
	added_frame->Add_Message( shared_ptr< const IProcessMessage >( new CAddMailboxMessage( ui_conn->Get_Writable_Mailbox() ) ) );
	process_tester.Get_Self_Proxy()->Get_Writable_Mailbox()->Add_Frame( added_frame );

	// generate a message that goes nowhere
	shared_ptr< CScheduledTask > db_task( new CSendMailboxMessageTask( 0.0, ui_conn->Get_Writable_Mailbox(), DB_PROCESS_ID ) );
	process_tester.Get_Process()->Get_Task_Scheduler()->Submit_Task( db_task );

	process_tester.Service( 0.0 );

	// verify log interface added
	auto mailboxes = process_tester.Get_Mailbox_Table();
	ASSERT_TRUE( mailboxes.size() == 1 );	
	ASSERT_TRUE( mailboxes.find( UI_PROCESS_ID ) != mailboxes.end() );

	// verify pending message that was not able to be sent
	auto frame_table = process_tester.Get_Frame_Table();
	ASSERT_TRUE( frame_table.size() == 1 );
	ASSERT_TRUE( frame_table.find( DB_PROCESS_ID ) != frame_table.end() );

	shared_ptr< CProcessMessageFrame > frame = frame_table.find( DB_PROCESS_ID )->second;
	ASSERT_TRUE( frame->Get_Process_ID() == tester_id );
	for ( auto iter = frame->Get_Frame_Begin(); iter != frame->Get_Frame_End(); ++iter )
	{
		const IProcessMessage *raw_message = iter->get();

		ASSERT_TRUE( Loki::TypeInfo( typeid( *raw_message ) ) == Loki::TypeInfo( typeid( CAddMailboxMessage ) ) );

		const CAddMailboxMessage *add_message = static_cast< const CAddMailboxMessage * >( raw_message );

		ASSERT_TRUE( add_message->Get_Mailbox().get() == ui_conn->Get_Writable_Mailbox().get() );
	}
	
	// generate a message to the ui thread which should go through
	shared_ptr< CScheduledTask > simple_task( new CSendMailboxMessageTask( 1.0, log_conn->Get_Writable_Mailbox(), UI_PROCESS_ID ) );
	process_tester.Get_Process()->Get_Task_Scheduler()->Submit_Task( simple_task );

	// shutdown both a known interface (UI_PROCESS_ID) and an unknown interface (DB_PROCESS_ID)
	shared_ptr< CProcessMessageFrame > shutdown_frame( new CProcessMessageFrame( MANAGER_PROCESS_ID ) );
	shutdown_frame->Add_Message( shared_ptr< const IProcessMessage >( new CReleaseMailboxRequest( UI_PROCESS_ID ) ) );
	shutdown_frame->Add_Message( shared_ptr< const IProcessMessage >( new CReleaseMailboxRequest( DB_PROCESS_ID ) ) );
	process_tester.Get_Self_Proxy()->Get_Writable_Mailbox()->Add_Frame( shutdown_frame );

	process_tester.Service( 1.0 );

	// verify interfaces released
	mailboxes = process_tester.Get_Mailbox_Table();
	ASSERT_TRUE( mailboxes.size() == 0 );	

	// no pending messages now that we added the double flush
	ASSERT_TRUE( process_tester.Get_Frame_Table().size() == 0 );

	// verify ui message reached destination
	std::vector< shared_ptr< CProcessMessageFrame > > frames;
	ui_conn->Get_Readable_Mailbox()->Remove_Frames( frames );

	ASSERT_TRUE( frames.size() == 1 );

	frame = frames[ 0 ];
	for ( auto iter = frame->Get_Frame_Begin(); iter != frame->Get_Frame_End(); ++iter )
	{
		const IProcessMessage *raw_message = iter->get();

		ASSERT_TRUE( Loki::TypeInfo( typeid( *raw_message ) ) == Loki::TypeInfo( typeid( CAddMailboxMessage ) ) );

		const CAddMailboxMessage *add_message = static_cast< const CAddMailboxMessage * >( raw_message );

		ASSERT_TRUE( add_message->Get_Mailbox().get() == log_conn->Get_Writable_Mailbox().get() );
	}

	// verify only pending messages are shutdown acknowledgements
	process_tester.Get_Manager_Proxy()->Get_Readable_Mailbox()->Remove_Frames( frames );
	for ( auto outer_iter = frames.cbegin(); outer_iter != frames.cend(); ++outer_iter )
	{
		frame = *outer_iter;
		ASSERT_TRUE( frame->Get_Process_ID() == AI_PROCESS_ID );

		for ( auto iter = frame->Get_Frame_Begin(); iter != frame->Get_Frame_End(); ++iter )
		{
			const IProcessMessage *raw_message = iter->get();

			if ( Loki::TypeInfo( typeid( *raw_message ) ) == Loki::TypeInfo( typeid( CReleaseMailboxResponse ) ) )
			{
				const CReleaseMailboxResponse *ack_message = static_cast< const CReleaseMailboxResponse * >( raw_message );

				ASSERT_TRUE( ack_message->Get_Shutdown_Process_ID() == UI_PROCESS_ID || ack_message->Get_Shutdown_Process_ID() == DB_PROCESS_ID );
			}
			else
			{
				ASSERT_TRUE( Loki::TypeInfo( typeid( *raw_message ) ) == Loki::TypeInfo( typeid( CRescheduleProcessMessage ) ) );
			}
		}
	}
}

TEST_F( ProcessTests, Shutdown_Soft )
{
	static const std::wstring LOG_MESSAGE( L"Blah blah" );

	CTaskProcessBaseTester process_tester( new CTestProcessTask( AI_PROPS ) );

	shared_ptr< CProcessMailbox > ui_conn( new CProcessMailbox( UI_PROCESS_ID, UI_PROPS ) );
	shared_ptr< CProcessMailbox > log_conn( new CProcessMailbox( LOGGING_PROCESS_ID, LOGGING_PROCESS_PROPERTIES ) );
	process_tester.Set_Logging_Mailbox( log_conn->Get_Writable_Mailbox() );

	shared_ptr< CProcessMessageFrame > added_frame( new CProcessMessageFrame( MANAGER_PROCESS_ID ) );

	added_frame->Add_Message( shared_ptr< const IProcessMessage >( new CAddMailboxMessage( ui_conn->Get_Writable_Mailbox() ) ) );
	added_frame->Add_Message( shared_ptr< const IProcessMessage >( new CReleaseMailboxRequest( UI_PROCESS_ID ) ) );
	added_frame->Add_Message( shared_ptr< const IProcessMessage >( new CShutdownSelfRequest( false ) ) );

	process_tester.Get_Self_Proxy()->Get_Writable_Mailbox()->Add_Frame( added_frame );

	// message going nowhere
	shared_ptr< CScheduledTask > db_task( new CSendMailboxMessageTask( 0.0, log_conn->Get_Writable_Mailbox(), DB_PROCESS_ID ) );
	process_tester.Get_Process()->Get_Task_Scheduler()->Submit_Task( db_task );

	// message to ui thread
	shared_ptr< CScheduledTask > ui_task( new CSendMailboxMessageTask( 0.0, log_conn->Get_Writable_Mailbox(), UI_PROCESS_ID ) );
	process_tester.Get_Process()->Get_Task_Scheduler()->Submit_Task( ui_task );

	// log message
	process_tester.Log( LOG_MESSAGE );

	process_tester.Service( 0.0 );

	// thread should have no outbound frames
	auto outbound_frames = process_tester.Get_Frame_Table();
	ASSERT_TRUE( outbound_frames.size() == 0 );

	// thread should have no interfaces
	auto mailboxes = process_tester.Get_Mailbox_Table();
	ASSERT_TRUE( mailboxes.size() == 0 );	

	// verify a single log message sent
	std::vector< shared_ptr< CProcessMessageFrame > > frames;
	log_conn->Get_Readable_Mailbox()->Remove_Frames( frames );

	ASSERT_TRUE( frames.size() == 1 );

	auto frame = frames[ 0 ];
	for ( auto iter = frame->Get_Frame_Begin(); iter != frame->Get_Frame_End(); ++iter )
	{
		const IProcessMessage *raw_message = iter->get();

		ASSERT_TRUE( Loki::TypeInfo( typeid( *raw_message ) ) == Loki::TypeInfo( typeid( CLogRequestMessage ) ) );

		const CLogRequestMessage *log_message = static_cast< const CLogRequestMessage * >( raw_message );

		ASSERT_TRUE( log_message->Get_Source_Properties() == AI_PROPS );
		ASSERT_TRUE( log_message->Get_Message() == LOG_MESSAGE );
	}

	// verify the ui message sent
	ui_conn->Get_Readable_Mailbox()->Remove_Frames( frames );

	ASSERT_TRUE( frames.size() == 1 );

	frame = frames[ 0 ];
	for ( auto iter = frame->Get_Frame_Begin(); iter != frame->Get_Frame_End(); ++iter )
	{
		const IProcessMessage *raw_message = iter->get();

		ASSERT_TRUE( Loki::TypeInfo( typeid( *raw_message ) ) == Loki::TypeInfo( typeid( CAddMailboxMessage ) ) );

		const CAddMailboxMessage *add_message = static_cast< const CAddMailboxMessage * >( raw_message );

		ASSERT_TRUE( add_message->Get_Mailbox().get() == log_conn->Get_Writable_Mailbox().get() );
	}

	// verify the acknowledgement messages sent to the manager
	process_tester.Get_Manager_Proxy()->Get_Readable_Mailbox()->Remove_Frames( frames );

	for ( auto outer_iter = frames.cbegin(); outer_iter != frames.cend(); ++outer_iter )
	{
		frame = *outer_iter;
		ASSERT_TRUE( frame->Get_Process_ID() == AI_PROCESS_ID );

		for ( auto iter = frame->Get_Frame_Begin(); iter != frame->Get_Frame_End(); ++iter )
		{
			const IProcessMessage *raw_message = iter->get();

			if ( Loki::TypeInfo( typeid( *raw_message ) ) == Loki::TypeInfo( typeid( CReleaseMailboxResponse ) ) )
			{
				const CReleaseMailboxResponse *ack_message = static_cast< const CReleaseMailboxResponse * >( raw_message );

				ASSERT_TRUE( ack_message->Get_Shutdown_Process_ID() == UI_PROCESS_ID );
			}
			else
			{
				ASSERT_TRUE( Loki::TypeInfo( typeid( *raw_message ) ) == Loki::TypeInfo( typeid( CShutdownSelfResponse ) ) );
			}
		}
	}
}

TEST_F( ProcessTests, Shutdown_Hard )
{
	static const std::wstring LOG_MESSAGE( L"Hard shutdown" );

	CTaskProcessBaseTester process_tester( new CTestProcessTask( AI_PROPS ) );

	shared_ptr< CProcessMailbox > ui_conn( new CProcessMailbox( UI_PROCESS_ID, UI_PROPS ) );
	shared_ptr< CProcessMailbox > log_conn( new CProcessMailbox( LOGGING_PROCESS_ID, LOGGING_PROCESS_PROPERTIES ) );
	process_tester.Set_Logging_Mailbox( log_conn->Get_Writable_Mailbox() );

	shared_ptr< CProcessMessageFrame > added_frame( new CProcessMessageFrame( MANAGER_PROCESS_ID ) );

	added_frame->Add_Message( shared_ptr< const IProcessMessage >( new CAddMailboxMessage( ui_conn->Get_Writable_Mailbox() ) ) );
	added_frame->Add_Message( shared_ptr< const IProcessMessage >( new CReleaseMailboxRequest( UI_PROCESS_ID ) ) );
	added_frame->Add_Message( shared_ptr< const IProcessMessage >( new CShutdownSelfRequest( true ) ) );

	process_tester.Get_Self_Proxy()->Get_Writable_Mailbox()->Add_Frame( added_frame );

	// message going nowhere
	shared_ptr< CScheduledTask > db_task( new CSendMailboxMessageTask( 0.0, log_conn->Get_Writable_Mailbox(), DB_PROCESS_ID ) );
	process_tester.Get_Process()->Get_Task_Scheduler()->Submit_Task( db_task );

	// message to ui thread, but shouldn't get sent since it's a hard shutdown
	shared_ptr< CScheduledTask > ui_task( new CSendMailboxMessageTask( 0.0, log_conn->Get_Writable_Mailbox(), UI_PROCESS_ID ) );
	process_tester.Get_Process()->Get_Task_Scheduler()->Submit_Task( ui_task );

	// log message
	process_tester.Log( LOG_MESSAGE );

	process_tester.Service( 0.0 );

	// thread should have no outbound frames
	auto outbound_frames = process_tester.Get_Frame_Table();
	ASSERT_TRUE( outbound_frames.size() == 0 );

	// thread should have no interfaces
	auto mailboxes = process_tester.Get_Mailbox_Table();
	ASSERT_TRUE( mailboxes.size() == 0 );	

	// verify a single log message sent
	std::vector< shared_ptr< CProcessMessageFrame > > frames;
	log_conn->Get_Readable_Mailbox()->Remove_Frames( frames );

	ASSERT_TRUE( frames.size() == 1 );

	auto frame = frames[ 0 ];
	for ( auto iter = frame->Get_Frame_Begin(); iter != frame->Get_Frame_End(); ++iter )
	{
		const IProcessMessage *raw_message = iter->get();

		ASSERT_TRUE( Loki::TypeInfo( typeid( *raw_message ) ) == Loki::TypeInfo( typeid( CLogRequestMessage ) ) );

		const CLogRequestMessage *log_message = static_cast< const CLogRequestMessage * >( raw_message );

		ASSERT_TRUE( log_message->Get_Source_Properties() == AI_PROPS );
		ASSERT_TRUE( log_message->Get_Message() == LOG_MESSAGE );
	}

	// verify the ui message was not sent
	ui_conn->Get_Readable_Mailbox()->Remove_Frames( frames );
	ASSERT_TRUE( frames.size() == 0 );

	// verify the acknowledgement messages sent to the manager
	process_tester.Get_Manager_Proxy()->Get_Readable_Mailbox()->Remove_Frames( frames );

	for ( auto outer_iter = frames.cbegin(); outer_iter != frames.cend(); ++outer_iter )
	{
		frame = *outer_iter;
		ASSERT_TRUE( frame->Get_Process_ID() == AI_PROCESS_ID );

		for ( auto iter = frame->Get_Frame_Begin(); iter != frame->Get_Frame_End(); ++iter )
		{
			const IProcessMessage *raw_message = iter->get();

			if ( Loki::TypeInfo( typeid( *raw_message ) ) == Loki::TypeInfo( typeid( CReleaseMailboxResponse ) ) )
			{
				const CReleaseMailboxResponse *ack_message = static_cast< const CReleaseMailboxResponse * >( raw_message );

				ASSERT_TRUE( ack_message->Get_Shutdown_Process_ID() == UI_PROCESS_ID );
			}
			else
			{
				ASSERT_TRUE( Loki::TypeInfo( typeid( *raw_message ) ) == Loki::TypeInfo( typeid( CShutdownSelfResponse ) ) );
			}
		}
	}

}

TEST_F( ProcessTests, Thread_Shutdown )
{
	CTestThreadProcess *test_thread_process = new CTestThreadProcess( AI_PROPS );
	CThreadProcessBaseTester process_tester( test_thread_process );

	shared_ptr< CProcessMailbox > log_conn( new CProcessMailbox( LOGGING_PROCESS_ID, LOGGING_PROCESS_PROPERTIES ) );
	process_tester.Set_Logging_Mailbox( log_conn->Get_Writable_Mailbox() );

	shared_ptr< CPlatformThread > platform_thread( new CPlatformThread() );

	CProcessExecutionContext thread_context( platform_thread.get() );
	process_tester.Get_Thread_Process()->Run( thread_context );

	while( test_thread_process->Get_Frames_Completed() < 10 )
	{
		NPlatform::Sleep( 1 );
	}

	ASSERT_TRUE( platform_thread->Is_Running() );

	shared_ptr< CProcessMessageFrame > shutdown_frame( new CProcessMessageFrame( MANAGER_PROCESS_ID ) );
	shutdown_frame->Add_Message( shared_ptr< const IProcessMessage >( new CShutdownSelfRequest( false ) ) );

	process_tester.Get_Self_Proxy()->Get_Writable_Mailbox()->Add_Frame( shutdown_frame );

	while ( platform_thread->Is_Running() )
	{
		NPlatform::Sleep( 1 );
	}
}