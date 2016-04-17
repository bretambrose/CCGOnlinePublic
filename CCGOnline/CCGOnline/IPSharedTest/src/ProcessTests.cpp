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

#include <IPCore/Process/Process.h>
#include <IPShared/Concurrency/TaskProcessBase.h>
#include <IPShared/Concurrency/ThreadProcessBase.h>
#include <IPShared/Concurrency/ProcessMailbox.h>
#include <IPShared/Concurrency/ProcessConstants.h>
#include <IPShared/Concurrency/ProcessStatics.h>
#include <IPShared/Concurrency/ProcessExecutionContext.h>
#include <IPShared/Concurrency/MailboxInterfaces.h>
#include <IPShared/Concurrency/ProcessMessageFrame.h>
#include <IPShared/Concurrency/Messaging/ProcessManagementMessages.h>
#include <IPShared/Concurrency/Messaging/ExchangeMailboxMessages.h>
#include <IPShared/Concurrency/Messaging/LoggingMessages.h>
#include <IPShared/Concurrency/ProcessID.h>
#include <IPShared/TaskScheduler/ScheduledTask.h>
#include <IPShared/TaskScheduler/TaskScheduler.h>
#include <IPShared/Logging/LogInterface.h>
#include <IPSharedTest/Helpers/ProcessHelpers.h>
#include <IPSharedTest/SharedTestProcessSubject.h>
#include <gtest/gtest.h>
#include <loki/LokiTypeInfo.h>

using namespace IP::Execution;
using namespace IP::Execution::Messaging;
using namespace IP::Logging;

class ProcessTests : public testing::Test 
{
	protected:  


};

class CTestProcessTask : public CTaskProcessBase
{
	public:

		using BASECLASS = CTaskProcessBase;

		CTestProcessTask( const SProcessProperties &properties ) :
			BASECLASS( properties )
		{}

		virtual bool Is_Root_Thread( void ) const { return true; }

	protected:

	private:
};

class CTestThreadProcess : public CThreadProcessBase
{
	public:

		using BASECLASS = CThreadProcessBase;

		CTestThreadProcess( const SProcessProperties &properties ) :
			BASECLASS( properties ),
			FramesCompleted( 0 )
		{}

		virtual bool Is_Root_Thread( void ) const { return true; }

		uint64_t Get_Frames_Completed( void ) const { return FramesCompleted; }

	protected:

		virtual void Per_Frame_Logic_End( void ) { ++FramesCompleted; }

		virtual uint32_t Get_Sleep_Interval_In_Milliseconds( void ) const { return 10; }

	private:

		uint64_t FramesCompleted;
};


class CBasicServiceTestTask : public CScheduledTask
{
	public:

		using BASECLASS = CScheduledTask;

		CBasicServiceTestTask( double execute_time_seconds ) :
			BASECLASS( execute_time_seconds )
		{}

		virtual bool Execute( double /*time_seconds*/, double & /*reschedule_time_seconds*/ )
		{
			CTaskProcessBaseTester::Set_Has_Process_Service_Executed();

			return false;
		}
};

SProcessProperties AI_PROPS( ETestExtendedProcessSubject::AI  );



TEST_F( ProcessTests, Basic_Service_And_Reschedule )
{
	static const double FIRST_SERVICE_TIME = 1.0;
	static const double SECOND_SERVICE_TIME = 4.99;
	static const double THIRD_SERVICE_TIME = 5.0;

	CTaskProcessBaseTester process_tester( IP::New< CTestProcessTask >( MEMORY_TAG, AI_PROPS ) );

	ASSERT_FALSE( CTaskProcessBaseTester::Get_Has_Process_Service_Executed() );

	auto simple_task = IP::Make_Shared< CBasicServiceTestTask >( MEMORY_TAG, 5.0 );
	process_tester.Get_Process()->Get_Task_Scheduler()->Submit_Task( simple_task );

	process_tester.Service( FIRST_SERVICE_TIME );
	ASSERT_FALSE( CTaskProcessBaseTester::Get_Has_Process_Service_Executed() );

	IP::Vector< IP::UniquePtr< CProcessMessageFrame > > frames;
	process_tester.Get_Manager_Proxy()->Get_Readable_Mailbox()->Remove_Frames( frames );
	ASSERT_TRUE( frames.size() == 1 );

	for ( auto iter = frames[ 0 ]->cbegin(), end = frames[ 0 ]->cend(); iter != end; ++iter )
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

	for ( auto iter = frames[ 0 ]->cbegin(), end = frames[ 0 ]->cend(); iter != end; ++iter )
	{
		const IProcessMessage *raw_message = iter->get();

		ASSERT_TRUE( Loki::TypeInfo( typeid( *raw_message ) ) == Loki::TypeInfo( typeid( CRescheduleProcessMessage ) ) );

		const CRescheduleProcessMessage *reschedule_message = static_cast< const CRescheduleProcessMessage * >( raw_message );

		ASSERT_DOUBLE_EQ( reschedule_message->Get_Reschedule_Time(), THIRD_SERVICE_TIME );
	}

	process_tester.Service( THIRD_SERVICE_TIME );
	ASSERT_TRUE( CTaskProcessBaseTester::Get_Has_Process_Service_Executed() );
}



static const SProcessProperties DB_PROPS( ETestExtendedProcessSubject::DATABASE );
static const EProcessID DB_PROCESS_ID = static_cast< EProcessID >( static_cast< uint64_t >( EProcessID::FIRST_FREE_ID ) + 1 );

class CSendAddMailboxMessageServiceTask : public CScheduledTask
{
	public:

		using BASECLASS = CScheduledTask;

		CSendAddMailboxMessageServiceTask( double execute_time_seconds, const std::shared_ptr< CWriteOnlyMailbox > &mailbox ) :
			BASECLASS( execute_time_seconds ),
			Mailbox( mailbox )
		{}

		virtual bool Execute( double time_seconds, double &reschedule_time_seconds )
		{
			CProcessStatics::Get_Current_Process()->Send_Process_Message( DB_PROCESS_ID, IP::Make_Process_Message< CAddMailboxMessage >( MEMORY_TAG, Mailbox ) );

			reschedule_time_seconds = time_seconds + .1;
			return true;
		}

	private:

		std::shared_ptr< CWriteOnlyMailbox > Mailbox;
};


TEST_F( ProcessTests, Send_Message )
{
	CTaskProcessBaseTester process_tester( IP::New< CTestProcessTask >( MEMORY_TAG, AI_PROPS ) );

	auto simple_task = IP::Make_Shared< CSendAddMailboxMessageServiceTask >( MEMORY_TAG, 0.0, process_tester.Get_Self_Proxy()->Get_Writable_Mailbox() );
	process_tester.Get_Process()->Get_Task_Scheduler()->Submit_Task( simple_task );

	process_tester.Service( 1.0 );

	auto const &frame_table = process_tester.Get_Frame_Table();

	ASSERT_TRUE( frame_table.size() == 1 );
	ASSERT_TRUE( frame_table.find( DB_PROCESS_ID ) != frame_table.end() );

	const IP::UniquePtr< CProcessMessageFrame > &db_frame = process_tester.Get_Frame( DB_PROCESS_ID ); 

	for ( auto iter = db_frame->cbegin(), end = db_frame->cend(); iter != end; ++iter )
	{
		const IProcessMessage *raw_message = iter->get();

		ASSERT_TRUE( Loki::TypeInfo( typeid( *raw_message ) ) == Loki::TypeInfo( typeid( CAddMailboxMessage ) ) );

		const CAddMailboxMessage *add_mailbox_message = static_cast< const CAddMailboxMessage * >( raw_message );

		ASSERT_TRUE( add_mailbox_message->Get_Mailbox().get() == process_tester.Get_Self_Proxy()->Get_Writable_Mailbox().get() );
	}
		
	auto db_conn = IP::Make_Shared< CProcessMailbox >( MEMORY_TAG, DB_PROCESS_ID, DB_PROPS );

	// notify the thread of the db interface
	auto added_frame = IP::Make_Unique< CProcessMessageFrame >( MEMORY_TAG, EProcessID::CONCURRENCY_MANAGER );
	added_frame->Add_Message( IP::Make_Const_Process_Message< CAddMailboxMessage >( MEMORY_TAG, db_conn->Get_Writable_Mailbox() ) );
	process_tester.Get_Self_Proxy()->Get_Writable_Mailbox()->Add_Frame( added_frame );

	process_tester.Service( 2.0 );

	// verify new interface added
	auto interfaces = process_tester.Get_Mailbox_Table();
	ASSERT_TRUE( interfaces.size() == 1 );	
	ASSERT_TRUE( interfaces.find( DB_PROCESS_ID ) != interfaces.end() );

	// verify both (send task is recurrent) messages sent to the db thread's mailbox
	ASSERT_TRUE( frame_table.size() == 0 );

	IP::Vector< IP::UniquePtr< CProcessMessageFrame > > frames;
	db_conn->Get_Readable_Mailbox()->Remove_Frames( frames );

	ASSERT_TRUE( frames.size() == 1 );

	for ( auto iter = frames[ 0 ]->cbegin(), end = frames[ 0 ]->cend(); iter != end; ++iter )
	{
		const IProcessMessage *raw_message = iter->get();

		ASSERT_TRUE( Loki::TypeInfo( typeid( *raw_message ) ) == Loki::TypeInfo( typeid( CAddMailboxMessage ) ) );

		const CAddMailboxMessage *add_mailbox_message = static_cast< const CAddMailboxMessage * >( raw_message );

		ASSERT_TRUE( add_mailbox_message->Get_Mailbox().get() == process_tester.Get_Self_Proxy()->Get_Writable_Mailbox().get() );
	}
}

TEST_F( ProcessTests, Add_Mailbox_And_Logging )
{
	static const IP::String LOG_MESSAGE_1( "Log Test 1" );
	static const IP::String LOG_MESSAGE_2( "Log Test 2" );
	static const IP::String LOG_MESSAGE_3( "Log Test 3" );

	CTaskProcessBaseTester process_tester( IP::New< CTestProcessTask >( MEMORY_TAG, AI_PROPS ) );

	// vanishes because no context to route to
	CLogInterface::Log( LOG_MESSAGE_1 );

	process_tester.Log( LOG_MESSAGE_2 );

	// Verify pending message due to no interface
	const IP::UniquePtr< CProcessMessageFrame > &log_frame = process_tester.Get_Log_Frame();
	ASSERT_TRUE( log_frame.get() != nullptr );
	
	for ( auto iter = log_frame->cbegin(), end = log_frame->cend(); iter != end; ++iter )
	{
		const IProcessMessage *raw_message = iter->get();

		ASSERT_TRUE( Loki::TypeInfo( typeid( *raw_message ) ) == Loki::TypeInfo( typeid( CLogRequestMessage ) ) );

		const CLogRequestMessage *log_message = static_cast< const CLogRequestMessage * >( raw_message );

		ASSERT_TRUE( log_message->Get_Source_Properties() == AI_PROPS );
		ASSERT_TRUE( log_message->Get_Message() == LOG_MESSAGE_2 );
	}
	
	auto log_conn = IP::Make_Shared< CProcessMailbox >( MEMORY_TAG, LOGGING_PROCESS_ID, LOGGING_PROCESS_PROPERTIES );

	// notify the thread of the log interface
	process_tester.Set_Logging_Mailbox( log_conn->Get_Writable_Mailbox() );

	// verify new interface added
	ASSERT_TRUE( process_tester.Get_Mailbox_Table().size() == 0 );	// 
	ASSERT_TRUE( process_tester.Get_Logging_Mailbox().get() != nullptr );

	process_tester.Service( 2.0 );

	// verify second log message sent to the log thread's mailbox
	auto const &frame_table = process_tester.Get_Frame_Table();
	ASSERT_TRUE( frame_table.size() == 0 );

	IP::Vector< IP::UniquePtr< CProcessMessageFrame > > frames;
	log_conn->Get_Readable_Mailbox()->Remove_Frames( frames );

	ASSERT_TRUE( frames.size() == 1 );
	ASSERT_TRUE( frames[ 0 ]->Get_Process_ID() == AI_PROCESS_ID );
	for ( auto iter = frames[ 0 ]->cbegin(), end = frames[ 0 ]->cend(); iter != end; ++iter )
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
	ASSERT_TRUE( frame_table.size() == 0 );

	frames.clear();
	log_conn->Get_Readable_Mailbox()->Remove_Frames( frames );

	ASSERT_TRUE( frames.size() == 1 );

	for ( auto iter = frames[ 0 ]->cbegin(), end = frames[ 0 ]->cend(); iter != end; ++iter )
	{
		const IProcessMessage *raw_message = iter->get();

		ASSERT_TRUE( Loki::TypeInfo( typeid( *raw_message ) ) == Loki::TypeInfo( typeid( CLogRequestMessage ) ) );

		const CLogRequestMessage *log_message = static_cast< const CLogRequestMessage * >( raw_message );

		ASSERT_TRUE( log_message->Get_Source_Properties() == AI_PROPS );
		ASSERT_TRUE( log_message->Get_Message() == LOG_MESSAGE_3 );
	}

}

static const SProcessProperties UI_PROPS( ETestExtendedProcessSubject::UI );
static const EProcessID UI_PROCESS_ID = static_cast< EProcessID >( static_cast< uint64_t >( EProcessID::FIRST_FREE_ID ) + 2 );

class CSendMailboxMessageTask : public CScheduledTask
{
	public:

		using BASECLASS = CScheduledTask;

		CSendMailboxMessageTask( double execute_time_seconds, const std::shared_ptr< CWriteOnlyMailbox > &mailbox, EProcessID target_process_id ) :
			BASECLASS( execute_time_seconds ),
			Mailbox( mailbox ),
			TargetProcessID( target_process_id )
		{}

		virtual bool Execute( double /*time_seconds*/, double & /*reschedule_time_seconds*/ )
		{
			CProcessStatics::Get_Current_Process()->Send_Process_Message( TargetProcessID, IP::Make_Process_Message< CAddMailboxMessage >( MEMORY_TAG, Mailbox ) );
			return false;
		}

	private:

		std::shared_ptr< CWriteOnlyMailbox > Mailbox;
		EProcessID TargetProcessID;
};

TEST_F( ProcessTests, Shutdown_Interface )
{
	static const IP::String LOG_MESSAGE( "Log Test" );

	CTaskProcessBaseTester process_tester( IP::New< CTestProcessTask >( MEMORY_TAG, AI_PROPS ) );
	EProcessID tester_id = process_tester.Get_Process()->Get_ID();

	auto log_conn = IP::Make_Shared< CProcessMailbox >( MEMORY_TAG, LOGGING_PROCESS_ID, LOGGING_PROCESS_PROPERTIES );
	auto ui_conn = IP::Make_Shared< CProcessMailbox >( MEMORY_TAG, UI_PROCESS_ID, UI_PROPS );

	auto added_frame = IP::Make_Unique< CProcessMessageFrame >( MEMORY_TAG, EProcessID::CONCURRENCY_MANAGER );
	added_frame->Add_Message( IP::Make_Const_Process_Message< CAddMailboxMessage >( MEMORY_TAG, ui_conn->Get_Writable_Mailbox() ) );
	process_tester.Get_Self_Proxy()->Get_Writable_Mailbox()->Add_Frame( added_frame );

	// generate a message that goes nowhere
	auto db_task = IP::Make_Scheduled_Task< CSendMailboxMessageTask >( MEMORY_TAG, 0.0, ui_conn->Get_Writable_Mailbox(), DB_PROCESS_ID );
	process_tester.Get_Process()->Get_Task_Scheduler()->Submit_Task( db_task );

	process_tester.Service( 0.0 );

	// verify log interface added
	auto mailboxes = process_tester.Get_Mailbox_Table();
	ASSERT_TRUE( mailboxes.size() == 1 );	
	ASSERT_TRUE( mailboxes.find( UI_PROCESS_ID ) != mailboxes.end() );

	// verify pending message that was not able to be sent
	auto const &frame_table = process_tester.Get_Frame_Table();
	ASSERT_TRUE( frame_table.size() == 1 );
	ASSERT_TRUE( frame_table.find( DB_PROCESS_ID ) != frame_table.end() );

	const IP::UniquePtr< CProcessMessageFrame > &frame = process_tester.Get_Frame( DB_PROCESS_ID ); 
	ASSERT_TRUE( frame->Get_Process_ID() == tester_id );
	for ( auto iter = frame->cbegin(), end = frame->cend(); iter != end; ++iter )
	{
		const IProcessMessage *raw_message = iter->get();

		ASSERT_TRUE( Loki::TypeInfo( typeid( *raw_message ) ) == Loki::TypeInfo( typeid( CAddMailboxMessage ) ) );

		const CAddMailboxMessage *add_message = static_cast< const CAddMailboxMessage * >( raw_message );

		ASSERT_TRUE( add_message->Get_Mailbox().get() == ui_conn->Get_Writable_Mailbox().get() );
	}
	
	// generate a message to the ui thread which should go through
	auto simple_task = IP::Make_Scheduled_Task< CSendMailboxMessageTask >( MEMORY_TAG, 1.0, log_conn->Get_Writable_Mailbox(), UI_PROCESS_ID );
	process_tester.Get_Process()->Get_Task_Scheduler()->Submit_Task( simple_task );

	// shutdown both a known interface (UI_PROCESS_ID) and an unknown interface (DB_PROCESS_ID)
	auto shutdown_frame = IP::Make_Unique< CProcessMessageFrame >( MEMORY_TAG, MANAGER_PROCESS_ID );
	shutdown_frame->Add_Message( IP::Make_Const_Process_Message< CReleaseMailboxRequest >( MEMORY_TAG, UI_PROCESS_ID ) );
	shutdown_frame->Add_Message( IP::Make_Const_Process_Message< CReleaseMailboxRequest >( MEMORY_TAG, DB_PROCESS_ID ) );
	process_tester.Get_Self_Proxy()->Get_Writable_Mailbox()->Add_Frame( shutdown_frame );

	process_tester.Service( 1.0 );

	// verify interfaces released
	mailboxes = process_tester.Get_Mailbox_Table();
	ASSERT_TRUE( mailboxes.size() == 0 );	

	// no pending messages now that we added the double flush
	ASSERT_TRUE( process_tester.Get_Frame_Table().size() == 0 );

	// verify ui message reached destination
	IP::Vector< IP::UniquePtr< CProcessMessageFrame > > frames;
	ui_conn->Get_Readable_Mailbox()->Remove_Frames( frames );

	ASSERT_TRUE( frames.size() == 1 );

	for ( auto iter = frames[ 0 ]->cbegin(), end = frames[ 0 ]->cend(); iter != end; ++iter )
	{
		const IProcessMessage *raw_message = iter->get();

		ASSERT_TRUE( Loki::TypeInfo( typeid( *raw_message ) ) == Loki::TypeInfo( typeid( CAddMailboxMessage ) ) );

		const CAddMailboxMessage *add_message = static_cast< const CAddMailboxMessage * >( raw_message );

		ASSERT_TRUE( add_message->Get_Mailbox().get() == log_conn->Get_Writable_Mailbox().get() );
	}

	// verify only pending messages are shutdown acknowledgements
	frames.clear();
	process_tester.Get_Manager_Proxy()->Get_Readable_Mailbox()->Remove_Frames( frames );
	for ( auto outer_iter = frames.cbegin(), outer_end = frames.cend(); outer_iter != outer_end; ++outer_iter )
	{
		const IP::UniquePtr< CProcessMessageFrame > &frame = *outer_iter;
		ASSERT_TRUE( frame->Get_Process_ID() == AI_PROCESS_ID );

		for ( auto iter = frame->cbegin(), end = frame->cend(); iter != end; ++iter )
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
	static const IP::String LOG_MESSAGE( "Blah blah" );

	CTaskProcessBaseTester process_tester( IP::New< CTestProcessTask >( MEMORY_TAG, AI_PROPS ) );

	auto ui_conn = IP::Make_Shared< CProcessMailbox >( MEMORY_TAG, UI_PROCESS_ID, UI_PROPS );
	auto log_conn = IP::Make_Shared< CProcessMailbox >( MEMORY_TAG, LOGGING_PROCESS_ID, LOGGING_PROCESS_PROPERTIES );
	process_tester.Set_Logging_Mailbox( log_conn->Get_Writable_Mailbox() );

	auto added_frame = IP::Make_Unique< CProcessMessageFrame >( MEMORY_TAG, MANAGER_PROCESS_ID );

	added_frame->Add_Message( IP::Make_Const_Process_Message< CAddMailboxMessage >( MEMORY_TAG, ui_conn->Get_Writable_Mailbox() ) );
	added_frame->Add_Message( IP::Make_Const_Process_Message< CReleaseMailboxRequest >( MEMORY_TAG, UI_PROCESS_ID ) );
	added_frame->Add_Message( IP::Make_Const_Process_Message< CShutdownSelfRequest >( MEMORY_TAG, false ) );

	process_tester.Get_Self_Proxy()->Get_Writable_Mailbox()->Add_Frame( added_frame );

	// message going nowhere
	auto db_task = IP::Make_Scheduled_Task< CSendMailboxMessageTask >( MEMORY_TAG, 0.0, log_conn->Get_Writable_Mailbox(), DB_PROCESS_ID );
	process_tester.Get_Process()->Get_Task_Scheduler()->Submit_Task( db_task );

	// message to ui thread
	auto ui_task = IP::Make_Scheduled_Task< CSendMailboxMessageTask >( MEMORY_TAG, 0.0, log_conn->Get_Writable_Mailbox(), UI_PROCESS_ID );
	process_tester.Get_Process()->Get_Task_Scheduler()->Submit_Task( ui_task );

	// log message
	process_tester.Log( LOG_MESSAGE );

	process_tester.Service( 0.0 );

	// thread should have no outbound frames
	auto const &outbound_frames = process_tester.Get_Frame_Table();
	ASSERT_TRUE( outbound_frames.size() == 0 );

	// thread should have no interfaces
	auto mailboxes = process_tester.Get_Mailbox_Table();
	ASSERT_TRUE( mailboxes.size() == 0 );	

	// verify a single log message sent
	IP::Vector< IP::UniquePtr< CProcessMessageFrame > > frames;
	log_conn->Get_Readable_Mailbox()->Remove_Frames( frames );

	ASSERT_TRUE( frames.size() == 1 );

	for ( auto iter = frames[ 0 ]->cbegin(), end = frames[ 0 ]->cend(); iter != end; ++iter )
	{
		const IProcessMessage *raw_message = iter->get();

		ASSERT_TRUE( Loki::TypeInfo( typeid( *raw_message ) ) == Loki::TypeInfo( typeid( CLogRequestMessage ) ) );

		const CLogRequestMessage *log_message = static_cast< const CLogRequestMessage * >( raw_message );

		ASSERT_TRUE( log_message->Get_Source_Properties() == AI_PROPS );
		ASSERT_TRUE( log_message->Get_Message() == LOG_MESSAGE );
	}

	// verify the ui message sent
	frames.clear();
	ui_conn->Get_Readable_Mailbox()->Remove_Frames( frames );

	ASSERT_TRUE( frames.size() == 1 );

	for ( auto iter = frames[ 0 ]->cbegin(), end = frames[ 0 ]->cend(); iter !=end; ++iter )
	{
		const IProcessMessage *raw_message = iter->get();

		ASSERT_TRUE( Loki::TypeInfo( typeid( *raw_message ) ) == Loki::TypeInfo( typeid( CAddMailboxMessage ) ) );

		const CAddMailboxMessage *add_message = static_cast< const CAddMailboxMessage * >( raw_message );

		ASSERT_TRUE( add_message->Get_Mailbox().get() == log_conn->Get_Writable_Mailbox().get() );
	}

	// verify the acknowledgement messages sent to the manager
	frames.clear();
	process_tester.Get_Manager_Proxy()->Get_Readable_Mailbox()->Remove_Frames( frames );

	for ( auto outer_iter = frames.cbegin(), outer_end = frames.cend(); outer_iter != outer_end; ++outer_iter )
	{
		const IP::UniquePtr< CProcessMessageFrame > &frame = *outer_iter;
		ASSERT_TRUE( frame->Get_Process_ID() == AI_PROCESS_ID );

		for ( auto iter = frame->cbegin(), end = frame->cend(); iter != end; ++iter )
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
	static const IP::String LOG_MESSAGE( "Hard shutdown" );

	CTaskProcessBaseTester process_tester( IP::New< CTestProcessTask >( MEMORY_TAG, AI_PROPS ) );

	auto ui_conn = IP::Make_Shared< CProcessMailbox >( MEMORY_TAG, UI_PROCESS_ID, UI_PROPS );
	auto log_conn = IP::Make_Shared< CProcessMailbox >( MEMORY_TAG, LOGGING_PROCESS_ID, LOGGING_PROCESS_PROPERTIES );
	process_tester.Set_Logging_Mailbox( log_conn->Get_Writable_Mailbox() );

	auto added_frame = IP::Make_Unique< CProcessMessageFrame >( MEMORY_TAG, MANAGER_PROCESS_ID );

	added_frame->Add_Message( IP::Make_Const_Process_Message< CAddMailboxMessage >( MEMORY_TAG, ui_conn->Get_Writable_Mailbox() ) );
	added_frame->Add_Message( IP::Make_Const_Process_Message< CReleaseMailboxRequest >( MEMORY_TAG, UI_PROCESS_ID ) );
	added_frame->Add_Message( IP::Make_Const_Process_Message< CShutdownSelfRequest >( MEMORY_TAG, true ) );

	process_tester.Get_Self_Proxy()->Get_Writable_Mailbox()->Add_Frame( added_frame );

	// message going nowhere
	auto db_task = IP::Make_Scheduled_Task< CSendMailboxMessageTask >( MEMORY_TAG, 0.0, log_conn->Get_Writable_Mailbox(), DB_PROCESS_ID );
	process_tester.Get_Process()->Get_Task_Scheduler()->Submit_Task( db_task );

	// message to ui thread, but shouldn't get sent since it's a hard shutdown
	auto ui_task = IP::Make_Scheduled_Task< CSendMailboxMessageTask >( MEMORY_TAG, 0.0, log_conn->Get_Writable_Mailbox(), UI_PROCESS_ID );
	process_tester.Get_Process()->Get_Task_Scheduler()->Submit_Task( ui_task );

	// log message
	process_tester.Log( LOG_MESSAGE );

	process_tester.Service( 0.0 );

	// thread should have no outbound frames
	auto const &outbound_frames = process_tester.Get_Frame_Table();
	ASSERT_TRUE( outbound_frames.size() == 0 );

	// thread should have no interfaces
	auto mailboxes = process_tester.Get_Mailbox_Table();
	ASSERT_TRUE( mailboxes.size() == 0 );	

	// verify a single log message sent
	IP::Vector< IP::UniquePtr< CProcessMessageFrame > > frames;
	log_conn->Get_Readable_Mailbox()->Remove_Frames( frames );

	ASSERT_TRUE( frames.size() == 1 );

	for ( auto iter = frames[ 0 ]->cbegin(), end = frames[ 0 ]->cend(); iter != end; ++iter )
	{
		const IProcessMessage *raw_message = iter->get();

		ASSERT_TRUE( Loki::TypeInfo( typeid( *raw_message ) ) == Loki::TypeInfo( typeid( CLogRequestMessage ) ) );

		const CLogRequestMessage *log_message = static_cast< const CLogRequestMessage * >( raw_message );

		ASSERT_TRUE( log_message->Get_Source_Properties() == AI_PROPS );
		ASSERT_TRUE( log_message->Get_Message() == LOG_MESSAGE );
	}

	// verify the ui message was not sent
	frames.clear();
	ui_conn->Get_Readable_Mailbox()->Remove_Frames( frames );
	ASSERT_TRUE( frames.size() == 0 );

	// verify the acknowledgement messages sent to the manager
	frames.clear();
	process_tester.Get_Manager_Proxy()->Get_Readable_Mailbox()->Remove_Frames( frames );

	for ( auto outer_iter = frames.cbegin(), outer_end = frames.cend(); outer_iter != outer_end; ++outer_iter )
	{
		const IP::UniquePtr< CProcessMessageFrame > &frame = *outer_iter;
		ASSERT_TRUE( frame->Get_Process_ID() == AI_PROCESS_ID );

		for ( auto iter = frame->cbegin(), end = frame->cend(); iter != end; ++iter )
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
	auto test_thread_process = IP::Make_Shared< CTestThreadProcess >( MEMORY_TAG, AI_PROPS );
	CThreadProcessBaseTester process_tester( std::static_pointer_cast< CThreadProcessBase >( test_thread_process ) );

	auto log_conn = IP::Make_Shared< CProcessMailbox >( MEMORY_TAG, LOGGING_PROCESS_ID, LOGGING_PROCESS_PROPERTIES );
	process_tester.Set_Logging_Mailbox( log_conn->Get_Writable_Mailbox() );

	process_tester.Start();

	while( test_thread_process->Get_Frames_Completed() < 10 )
	{
		std::this_thread::sleep_for( std::chrono::seconds( 1 ) );
	}

	auto shutdown_frame = IP::Make_Unique< CProcessMessageFrame >( MEMORY_TAG, MANAGER_PROCESS_ID );
	shutdown_frame->Add_Message( IP::Make_Const_Process_Message< CShutdownSelfRequest >( MEMORY_TAG, false ) );

	process_tester.Get_Self_Proxy()->Get_Writable_Mailbox()->Add_Frame( shutdown_frame );

	test_thread_process->Finalize();
}


