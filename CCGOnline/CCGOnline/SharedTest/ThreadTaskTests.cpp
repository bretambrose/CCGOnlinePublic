/**********************************************************************************************************************

	ThreadTaskTests.cpp
		defines unit tests for thread task related functionality

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

#include "Concurrency/ThreadTaskBase.h"
#include "Concurrency/ThreadSubject.h"
#include "Concurrency/ThreadConnection.h"
#include "Concurrency/ThreadConstants.h"
#include "Concurrency/ThreadStatics.h"
#include "Concurrency/ThreadTaskExecutionContext.h"
#include "Concurrency/ThreadInterfaces.h"
#include "Concurrency/ThreadMessageFrame.h"
#include "Concurrency/ThreadMessages/ThreadManagementMessages.h"
#include "Concurrency/ThreadMessages/ExchangeInterfaceMessages.h"
#include "Concurrency/ThreadMessages/LoggingMessages.h"
#include "Time/TimeType.h"
#include "TaskScheduler/ScheduledTask.h"
#include "TaskScheduler/TaskScheduler.h"
#include "Logging/LogInterface.h"

class ThreadTaskTests : public testing::Test 
{
	protected:  


};

class CTestThreadTask : public CThreadTaskBase
{
	public:

		typedef CThreadTaskBase BASECLASS;

		CTestThreadTask( const SThreadKey &key ) :
			BASECLASS( key )
		{}

		virtual ETimeType Get_Time_Type( void ) const { return TT_GAME_TIME; }
		virtual bool Is_Root_Thread( void ) const { return true; }

	protected:

	private:
};

class CThreadTaskBaseTester
{
	public:

		CThreadTaskBaseTester( CThreadTaskBase *thread_task ) :
			ThreadTask( thread_task ),
			ManagerProxy( new CThreadConnection( MANAGER_THREAD_KEY ) ),
			SelfProxy( new CThreadConnection( thread_task->Get_Key() ) )
		{
			thread_task->Set_Manager_Interface( ManagerProxy->Get_Writer_Interface() );
			thread_task->Set_Read_Interface( SelfProxy->Get_Reader_Interface() );
			thread_task->Initialize();
		}

		~CThreadTaskBaseTester()
		{
		}

		void Service( double time_seconds )
		{
			CThreadStatics::Set_Current_Thread_Task( ThreadTask.get() );

			CThreadTaskExecutionContext context( nullptr );
			ThreadTask->Service( time_seconds, context );

			CThreadStatics::Set_Current_Thread_Task( nullptr );

			ThreadTask->Flush_Partitioned_Messages();
		}

		void Log( const std::wstring &log_string )
		{
			CThreadStatics::Set_Current_Thread_Task( ThreadTask.get() );
			CLogInterface::Log( log_string );
			CThreadStatics::Set_Current_Thread_Task( nullptr );
		}

		shared_ptr< CThreadTaskBase > Get_Thread_Task( void ) const { return ThreadTask; }

		shared_ptr< CThreadConnection > Get_Manager_Proxy( void ) const { return ManagerProxy; }
		shared_ptr< CThreadConnection > Get_Self_Proxy( void ) const { return SelfProxy; }

		double Get_Reschedule_Interval( void ) const { return ThreadTask->Get_Reschedule_Interval(); }

		static void Set_Has_Task_Service_Executed( void ) { HasTaskServiceExecuted = true; }
		static bool Get_Has_Task_Service_Executed( void ) { return HasTaskServiceExecuted; }

		const CThreadTaskBase::FrameTableType &Get_Frame_Table( void ) const { return ThreadTask->PendingOutboundFrames; }
		const CThreadTaskBase::InterfaceTable &Get_Interface_Table( void ) const { return ThreadTask->Interfaces; }

		shared_ptr< CWriteOnlyThreadInterface > Get_Log_Interface( void ) const { return ThreadTask->LogInterface; }
		shared_ptr< CWriteOnlyThreadInterface > Get_Manager_Interface( void ) const { return ThreadTask->ManagerInterface; }

		shared_ptr< CThreadMessageFrame > Get_Log_Frame( void ) const { return ThreadTask->LogFrame; }
		shared_ptr< CThreadMessageFrame > Get_Manager_Frame( void ) const { return ThreadTask->ManagerFrame; }

	private:

		static bool HasTaskServiceExecuted;

		shared_ptr< CThreadTaskBase > ThreadTask;

		shared_ptr< CThreadConnection > ManagerProxy;
		shared_ptr< CThreadConnection > SelfProxy;
};

bool CThreadTaskBaseTester::HasTaskServiceExecuted = false;

class CBasicServiceTestTask : public CScheduledTask
{
	public:

		typedef CScheduledTask BASECLASS;

		CBasicServiceTestTask( double execute_time_seconds ) :
			BASECLASS( execute_time_seconds )
		{}

		virtual bool Execute( double /*time_seconds*/, double & /*reschedule_time_seconds*/ )
		{
			CThreadTaskBaseTester::Set_Has_Task_Service_Executed();

			return false;
		}
};

TEST_F( ThreadTaskTests, Basic_Service_And_Reschedule )
{
	static const double FIRST_SERVICE_TIME = 1.0;
	static const double SECOND_SERVICE_TIME = 4.99;
	static const double THIRD_SERVICE_TIME = 5.0;

	SThreadKey test_key( TS_AI, 1, 1  );
	CThreadTaskBaseTester thread_tester( new CTestThreadTask( test_key ) );

	ASSERT_FALSE( CThreadTaskBaseTester::Get_Has_Task_Service_Executed() );

	shared_ptr< CScheduledTask > simple_task( new CBasicServiceTestTask( 5.0 ) );
	thread_tester.Get_Thread_Task()->Get_Task_Scheduler()->Submit_Task( simple_task );

	thread_tester.Service( FIRST_SERVICE_TIME );
	ASSERT_FALSE( CThreadTaskBaseTester::Get_Has_Task_Service_Executed() );

	std::vector< shared_ptr< CThreadMessageFrame > > frames;
	thread_tester.Get_Manager_Proxy()->Get_Reader_Interface()->Remove_Frames( frames );
	ASSERT_TRUE( frames.size() == 1 );

	shared_ptr< CThreadMessageFrame > frame = frames[ 0 ];
	for ( auto iter = frame->Get_Frame_Begin(); iter != frame->Get_Frame_End(); ++iter )
	{
		const IThreadMessage *raw_message = iter->get();

		ASSERT_TRUE( Loki::TypeInfo( typeid( *raw_message ) ) == Loki::TypeInfo( typeid( CRescheduleThreadMessage ) ) );

		const CRescheduleThreadMessage *reschedule_message = static_cast< const CRescheduleThreadMessage * >( raw_message );

		ASSERT_TRUE( reschedule_message->Get_Key() == test_key );
		ASSERT_DOUBLE_EQ( reschedule_message->Get_Reschedule_Time(), FIRST_SERVICE_TIME + thread_tester.Get_Reschedule_Interval() );
	}

	frames.clear();

	thread_tester.Service( SECOND_SERVICE_TIME );
	ASSERT_FALSE( CThreadTaskBaseTester::Get_Has_Task_Service_Executed() );

	thread_tester.Get_Manager_Proxy()->Get_Reader_Interface()->Remove_Frames( frames );
	ASSERT_TRUE( frames.size() == 1 );

	frame = frames[ 0 ];
	for ( auto iter = frame->Get_Frame_Begin(); iter != frame->Get_Frame_End(); ++iter )
	{
		const IThreadMessage *raw_message = iter->get();

		ASSERT_TRUE( Loki::TypeInfo( typeid( *raw_message ) ) == Loki::TypeInfo( typeid( CRescheduleThreadMessage ) ) );

		const CRescheduleThreadMessage *reschedule_message = static_cast< const CRescheduleThreadMessage * >( raw_message );

		ASSERT_TRUE( reschedule_message->Get_Key() == test_key );
		ASSERT_DOUBLE_EQ( reschedule_message->Get_Reschedule_Time(), THIRD_SERVICE_TIME );
	}

	thread_tester.Service( THIRD_SERVICE_TIME );
	ASSERT_TRUE( CThreadTaskBaseTester::Get_Has_Task_Service_Executed() );
}

static const SThreadKey DB_KEY( TS_DATABASE, 1, 1  );

class CSendMessageServiceTask : public CScheduledTask
{
	public:

		typedef CScheduledTask BASECLASS;

		CSendMessageServiceTask( double execute_time_seconds ) :
			BASECLASS( execute_time_seconds )
		{}

		virtual bool Execute( double time_seconds, double &reschedule_time_seconds )
		{
			CThreadStatics::Get_Current_Thread_Task()->
				Send_Thread_Message( DB_KEY, shared_ptr< const IThreadMessage >( new CAddInterfaceMessage( LOG_THREAD_KEY, shared_ptr< CWriteOnlyThreadInterface >( nullptr ) ) ) );

			reschedule_time_seconds = time_seconds + .1;
			return true;
		}
};

TEST_F( ThreadTaskTests, Send_Message )
{
	SThreadKey test_key( TS_AI, 1, 1  );
	CThreadTaskBaseTester thread_tester( new CTestThreadTask( test_key ) );

	shared_ptr< CScheduledTask > simple_task( new CSendMessageServiceTask( 0.0 ) );
	thread_tester.Get_Thread_Task()->Get_Task_Scheduler()->Submit_Task( simple_task );

	thread_tester.Service( 1.0 );

	// Verify pending message due to no interface
	auto frame_table = thread_tester.Get_Frame_Table();
	ASSERT_TRUE( frame_table.find( DB_KEY ) != frame_table.end() );
	shared_ptr< CThreadMessageFrame > db_frame = frame_table.find( DB_KEY )->second;
	for ( auto iter = db_frame->Get_Frame_Begin(); iter != db_frame->Get_Frame_End(); ++iter )
	{
		const IThreadMessage *raw_message = iter->get();

		ASSERT_TRUE( Loki::TypeInfo( typeid( *raw_message ) ) == Loki::TypeInfo( typeid( CAddInterfaceMessage ) ) );

		const CAddInterfaceMessage *add_interface_message = static_cast< const CAddInterfaceMessage * >( raw_message );

		ASSERT_TRUE( add_interface_message->Get_Key() == LOG_THREAD_KEY );
		ASSERT_TRUE( add_interface_message->Get_Interface().get() == nullptr );
	}

	
	shared_ptr< CThreadConnection > db_conn( new CThreadConnection( DB_KEY ) );

	// notify the thread of the db interface
	shared_ptr< CThreadMessageFrame > added_frame( new CThreadMessageFrame( MANAGER_THREAD_KEY ) );
	added_frame->Add_Message( shared_ptr< const IThreadMessage >( new CAddInterfaceMessage( DB_KEY, db_conn->Get_Writer_Interface() ) ) );
	thread_tester.Get_Self_Proxy()->Get_Writer_Interface()->Add_Frame( added_frame );

	thread_tester.Service( 2.0 );

	// verify new interface added
	auto interfaces = thread_tester.Get_Interface_Table();
	ASSERT_TRUE( interfaces.size() == 1 );	
	ASSERT_TRUE( interfaces.find( DB_KEY ) != interfaces.end() );

	// verify both messages sent to the db thread's mailbox
	frame_table = thread_tester.Get_Frame_Table();
	ASSERT_TRUE( frame_table.size() == 0 );

	std::vector< shared_ptr< CThreadMessageFrame > > frames;
	db_conn->Get_Reader_Interface()->Remove_Frames( frames );

	ASSERT_TRUE( frames.size() == 1 );

	shared_ptr< CThreadMessageFrame > frame = frames[ 0 ];
	for ( auto iter = frame->Get_Frame_Begin(); iter != frame->Get_Frame_End(); ++iter )
	{
		const IThreadMessage *raw_message = iter->get();

		ASSERT_TRUE( Loki::TypeInfo( typeid( *raw_message ) ) == Loki::TypeInfo( typeid( CAddInterfaceMessage ) ) );

		const CAddInterfaceMessage *add_interface_message = static_cast< const CAddInterfaceMessage * >( raw_message );

		ASSERT_TRUE( add_interface_message->Get_Key() == LOG_THREAD_KEY );
		ASSERT_TRUE( add_interface_message->Get_Interface().get() == nullptr );
	}
}

TEST_F( ThreadTaskTests, Add_Interface_And_Logging )
{
	static const std::wstring LOG_MESSAGE_1( L"Log Test 1" );
	static const std::wstring LOG_MESSAGE_2( L"Log Test 2" );
	static const std::wstring LOG_MESSAGE_3( L"Log Test 3" );

	SThreadKey test_key( TS_AI, 1, 1  );
	CThreadTaskBaseTester thread_tester( new CTestThreadTask( test_key ) );

	// vanishes because no context to route to
	CLogInterface::Log( LOG_MESSAGE_1 );

	thread_tester.Log( LOG_MESSAGE_2 );

	// Verify pending message due to no interface
	shared_ptr< CThreadMessageFrame > log_frame = thread_tester.Get_Log_Frame();
	ASSERT_TRUE( log_frame.get() != nullptr );
	
	for ( auto iter = log_frame->Get_Frame_Begin(); iter != log_frame->Get_Frame_End(); ++iter )
	{
		const IThreadMessage *raw_message = iter->get();

		ASSERT_TRUE( Loki::TypeInfo( typeid( *raw_message ) ) == Loki::TypeInfo( typeid( CLogRequestMessage ) ) );

		const CLogRequestMessage *log_message = static_cast< const CLogRequestMessage * >( raw_message );

		ASSERT_TRUE( log_message->Get_Source_Key() == test_key );
		ASSERT_TRUE( log_message->Get_Message() == LOG_MESSAGE_2 );
	}
	
	shared_ptr< CThreadConnection > log_conn( new CThreadConnection( LOG_THREAD_KEY ) );

	// notify the thread of the log interface
	shared_ptr< CThreadMessageFrame > added_frame( new CThreadMessageFrame( MANAGER_THREAD_KEY ) );
	added_frame->Add_Message( shared_ptr< const IThreadMessage >( new CAddInterfaceMessage( LOG_THREAD_KEY, log_conn->Get_Writer_Interface() ) ) );
	thread_tester.Get_Self_Proxy()->Get_Writer_Interface()->Add_Frame( added_frame );

	thread_tester.Service( 2.0 );

	// verify new interface added
	ASSERT_TRUE( thread_tester.Get_Interface_Table().size() == 0 );	// 
	ASSERT_TRUE( thread_tester.Get_Log_Interface().get() != nullptr );

	// verify second log message sent to the log thread's mailbox
	auto frame_table = thread_tester.Get_Frame_Table();
	ASSERT_TRUE( frame_table.size() == 0 );

	std::vector< shared_ptr< CThreadMessageFrame > > frames;
	log_conn->Get_Reader_Interface()->Remove_Frames( frames );

	ASSERT_TRUE( frames.size() == 1 );

	shared_ptr< CThreadMessageFrame > frame = frames[ 0 ];
	for ( auto iter = frame->Get_Frame_Begin(); iter != frame->Get_Frame_End(); ++iter )
	{
		const IThreadMessage *raw_message = iter->get();

		ASSERT_TRUE( Loki::TypeInfo( typeid( *raw_message ) ) == Loki::TypeInfo( typeid( CLogRequestMessage ) ) );

		const CLogRequestMessage *log_message = static_cast< const CLogRequestMessage * >( raw_message );

		ASSERT_TRUE( log_message->Get_Source_Key() == test_key );
		ASSERT_TRUE( log_message->Get_Message() == LOG_MESSAGE_2 );
	}

	thread_tester.Log( LOG_MESSAGE_3 );

	thread_tester.Service( 3.0 );

	// verify third log message sent to the log thread's mailbox
	frame_table = thread_tester.Get_Frame_Table();
	ASSERT_TRUE( frame_table.size() == 0 );

	log_conn->Get_Reader_Interface()->Remove_Frames( frames );

	ASSERT_TRUE( frames.size() == 1 );

	frame = frames[ 0 ];
	for ( auto iter = frame->Get_Frame_Begin(); iter != frame->Get_Frame_End(); ++iter )
	{
		const IThreadMessage *raw_message = iter->get();

		ASSERT_TRUE( Loki::TypeInfo( typeid( *raw_message ) ) == Loki::TypeInfo( typeid( CLogRequestMessage ) ) );

		const CLogRequestMessage *log_message = static_cast< const CLogRequestMessage * >( raw_message );

		ASSERT_TRUE( log_message->Get_Source_Key() == test_key );
		ASSERT_TRUE( log_message->Get_Message() == LOG_MESSAGE_3 );
	}

}

static const SThreadKey UI_KEY( TS_UI, 1, 1 );

class CSendUIMessageTask : public CScheduledTask
{
	public:

		typedef CScheduledTask BASECLASS;

		CSendUIMessageTask( double execute_time_seconds ) :
			BASECLASS( execute_time_seconds )
		{}

		virtual bool Execute( double /*time_seconds*/, double & /*reschedule_time_seconds*/ )
		{
			CThreadStatics::Get_Current_Thread_Task()->
				Send_Thread_Message( UI_KEY, shared_ptr< const IThreadMessage >( new CAddInterfaceMessage( LOG_THREAD_KEY, shared_ptr< CWriteOnlyThreadInterface >( nullptr ) ) ) );
			return false;
		}
};

class CSendDBMessageTask : public CScheduledTask
{
	public:

		typedef CScheduledTask BASECLASS;

		CSendDBMessageTask( double execute_time_seconds ) :
			BASECLASS( execute_time_seconds )
		{}

		virtual bool Execute( double /*time_seconds*/, double & /*reschedule_time_seconds*/ )
		{
			CThreadStatics::Get_Current_Thread_Task()->
				Send_Thread_Message( DB_KEY, shared_ptr< const IThreadMessage >( new CAddInterfaceMessage( LOG_THREAD_KEY, shared_ptr< CWriteOnlyThreadInterface >( nullptr ) ) ) );
			return false;
		}
};

TEST_F( ThreadTaskTests, Shutdown_Interface )
{
	static const std::wstring LOG_MESSAGE( L"Log Test" );

	SThreadKey test_key( TS_AI, 1, 1  );
	CThreadTaskBaseTester thread_tester( new CTestThreadTask( test_key ) );

	shared_ptr< CThreadConnection > ui_conn( new CThreadConnection( UI_KEY ) );

	shared_ptr< CThreadMessageFrame > added_frame( new CThreadMessageFrame( MANAGER_THREAD_KEY ) );
	added_frame->Add_Message( shared_ptr< const IThreadMessage >( new CAddInterfaceMessage( UI_KEY, ui_conn->Get_Writer_Interface() ) ) );
	thread_tester.Get_Self_Proxy()->Get_Writer_Interface()->Add_Frame( added_frame );

	// generate a message that goes nowhere
	shared_ptr< CScheduledTask > db_task( new CSendDBMessageTask( 0.0 ) );
	thread_tester.Get_Thread_Task()->Get_Task_Scheduler()->Submit_Task( db_task );

	thread_tester.Service( 0.0 );

	// verify log interface added
	auto interfaces = thread_tester.Get_Interface_Table();
	ASSERT_TRUE( interfaces.size() == 1 );	
	ASSERT_TRUE( interfaces.find( UI_KEY ) != interfaces.end() );

	// verify pending message that was not able to be sent
	auto frame_table = thread_tester.Get_Frame_Table();
	ASSERT_TRUE( frame_table.size() == 1 );
	ASSERT_TRUE( frame_table.find( DB_KEY ) != frame_table.end() );

	shared_ptr< CThreadMessageFrame > frame = frame_table.find( DB_KEY )->second;
	for ( auto iter = frame->Get_Frame_Begin(); iter != frame->Get_Frame_End(); ++iter )
	{
		const IThreadMessage *raw_message = iter->get();

		ASSERT_TRUE( Loki::TypeInfo( typeid( *raw_message ) ) == Loki::TypeInfo( typeid( CAddInterfaceMessage ) ) );

		const CAddInterfaceMessage *add_message = static_cast< const CAddInterfaceMessage * >( raw_message );

		ASSERT_TRUE( add_message->Get_Key() == LOG_THREAD_KEY );
		ASSERT_TRUE( add_message->Get_Interface().get() == nullptr );
	}
	
	// generate a message to the ui thread which should go through
	shared_ptr< CScheduledTask > simple_task( new CSendUIMessageTask( 1.0 ) );
	thread_tester.Get_Thread_Task()->Get_Task_Scheduler()->Submit_Task( simple_task );

	// shutdown both a known interface (UI_KEY) and an unknown interface (DB_KEY)
	shared_ptr< CThreadMessageFrame > shutdown_frame( new CThreadMessageFrame( MANAGER_THREAD_KEY ) );
	shutdown_frame->Add_Message( shared_ptr< const IThreadMessage >( new CShutdownInterfaceMessage( UI_KEY ) ) );
	shutdown_frame->Add_Message( shared_ptr< const IThreadMessage >( new CShutdownInterfaceMessage( DB_KEY ) ) );
	thread_tester.Get_Self_Proxy()->Get_Writer_Interface()->Add_Frame( shutdown_frame );

	thread_tester.Service( 1.0 );

	// verify interfaces released
	interfaces = thread_tester.Get_Interface_Table();
	ASSERT_TRUE( interfaces.size() == 0 );	

	// no pending messages now that we added the double flush
	ASSERT_TRUE( thread_tester.Get_Frame_Table().size() == 0 );

	// verify ui message reached destination
	std::vector< shared_ptr< CThreadMessageFrame > > frames;
	ui_conn->Get_Reader_Interface()->Remove_Frames( frames );

	ASSERT_TRUE( frames.size() == 1 );

	frame = frames[ 0 ];
	for ( auto iter = frame->Get_Frame_Begin(); iter != frame->Get_Frame_End(); ++iter )
	{
		const IThreadMessage *raw_message = iter->get();

		ASSERT_TRUE( Loki::TypeInfo( typeid( *raw_message ) ) == Loki::TypeInfo( typeid( CAddInterfaceMessage ) ) );

		const CAddInterfaceMessage *add_message = static_cast< const CAddInterfaceMessage * >( raw_message );

		ASSERT_TRUE( add_message->Get_Key() == LOG_THREAD_KEY );
		ASSERT_TRUE( add_message->Get_Interface().get() == nullptr );
	}

	// verify only pending messages are shutdown acknowledgements
	thread_tester.Get_Manager_Proxy()->Get_Reader_Interface()->Remove_Frames( frames );
	for ( auto outer_iter = frames.cbegin(); outer_iter != frames.cend(); ++outer_iter )
	{
		frame = *outer_iter;
		ASSERT_TRUE( frame->Get_Key() == test_key );

		for ( auto iter = frame->Get_Frame_Begin(); iter != frame->Get_Frame_End(); ++iter )
		{
			const IThreadMessage *raw_message = iter->get();

			if ( Loki::TypeInfo( typeid( *raw_message ) ) == Loki::TypeInfo( typeid( CShutdownInterfaceAcknowledgement ) ) )
			{
				const CShutdownInterfaceAcknowledgement *ack_message = static_cast< const CShutdownInterfaceAcknowledgement * >( raw_message );

				ASSERT_TRUE( ack_message->Get_Shutdown_Key() == UI_KEY || ack_message->Get_Shutdown_Key() == DB_KEY );
			}
			else
			{
				ASSERT_TRUE( Loki::TypeInfo( typeid( *raw_message ) ) == Loki::TypeInfo( typeid( CRescheduleThreadMessage ) ) );
			}
		}
	}
}

TEST_F( ThreadTaskTests, Shutdown_Soft )
{
	static const std::wstring LOG_MESSAGE( L"Blah blah" );

	SThreadKey test_key( TS_AI, 1, 1  );
	CThreadTaskBaseTester thread_tester( new CTestThreadTask( test_key ) );

	shared_ptr< CThreadConnection > ui_conn( new CThreadConnection( UI_KEY ) );
	shared_ptr< CThreadConnection > log_conn( new CThreadConnection( LOG_THREAD_KEY ) );

	shared_ptr< CThreadMessageFrame > added_frame( new CThreadMessageFrame( MANAGER_THREAD_KEY ) );

	added_frame->Add_Message( shared_ptr< const IThreadMessage >( new CAddInterfaceMessage( UI_KEY, ui_conn->Get_Writer_Interface() ) ) );
	added_frame->Add_Message( shared_ptr< const IThreadMessage >( new CAddInterfaceMessage( LOG_THREAD_KEY, log_conn->Get_Writer_Interface() ) ) );
	added_frame->Add_Message( shared_ptr< const IThreadMessage >( new CShutdownInterfaceMessage( UI_KEY ) ) );
	added_frame->Add_Message( shared_ptr< const IThreadMessage >( new CShutdownThreadRequest( false ) ) );

	thread_tester.Get_Self_Proxy()->Get_Writer_Interface()->Add_Frame( added_frame );

	// message going nowhere
	shared_ptr< CScheduledTask > db_task( new CSendDBMessageTask( 0.0 ) );
	thread_tester.Get_Thread_Task()->Get_Task_Scheduler()->Submit_Task( db_task );

	// message to ui thread
	shared_ptr< CScheduledTask > ui_task( new CSendUIMessageTask( 0.0 ) );
	thread_tester.Get_Thread_Task()->Get_Task_Scheduler()->Submit_Task( ui_task );

	// log message
	thread_tester.Log( LOG_MESSAGE );

	thread_tester.Service( 0.0 );

	// thread should have no outbound frames
	auto outbound_frames = thread_tester.Get_Frame_Table();
	ASSERT_TRUE( outbound_frames.size() == 0 );

	// thread should have no interfaces
	auto interfaces = thread_tester.Get_Interface_Table();
	ASSERT_TRUE( interfaces.size() == 0 );	

	// verify a single log message sent
	std::vector< shared_ptr< CThreadMessageFrame > > frames;
	log_conn->Get_Reader_Interface()->Remove_Frames( frames );

	ASSERT_TRUE( frames.size() == 1 );

	auto frame = frames[ 0 ];
	for ( auto iter = frame->Get_Frame_Begin(); iter != frame->Get_Frame_End(); ++iter )
	{
		const IThreadMessage *raw_message = iter->get();

		ASSERT_TRUE( Loki::TypeInfo( typeid( *raw_message ) ) == Loki::TypeInfo( typeid( CLogRequestMessage ) ) );

		const CLogRequestMessage *log_message = static_cast< const CLogRequestMessage * >( raw_message );

		ASSERT_TRUE( log_message->Get_Source_Key() == test_key );
		ASSERT_TRUE( log_message->Get_Message() == LOG_MESSAGE );
	}

	// verify the ui message sent
	ui_conn->Get_Reader_Interface()->Remove_Frames( frames );

	ASSERT_TRUE( frames.size() == 1 );

	frame = frames[ 0 ];
	for ( auto iter = frame->Get_Frame_Begin(); iter != frame->Get_Frame_End(); ++iter )
	{
		const IThreadMessage *raw_message = iter->get();

		ASSERT_TRUE( Loki::TypeInfo( typeid( *raw_message ) ) == Loki::TypeInfo( typeid( CAddInterfaceMessage ) ) );

		const CAddInterfaceMessage *add_message = static_cast< const CAddInterfaceMessage * >( raw_message );

		ASSERT_TRUE( add_message->Get_Key() == LOG_THREAD_KEY );
		ASSERT_TRUE( add_message->Get_Interface().get() == nullptr );
	}

	// verify the acknowledgement messages sent to the manager
	thread_tester.Get_Manager_Proxy()->Get_Reader_Interface()->Remove_Frames( frames );

	for ( auto outer_iter = frames.cbegin(); outer_iter != frames.cend(); ++outer_iter )
	{
		frame = *outer_iter;
		ASSERT_TRUE( frame->Get_Key() == test_key );

		for ( auto iter = frame->Get_Frame_Begin(); iter != frame->Get_Frame_End(); ++iter )
		{
			const IThreadMessage *raw_message = iter->get();

			if ( Loki::TypeInfo( typeid( *raw_message ) ) == Loki::TypeInfo( typeid( CShutdownInterfaceAcknowledgement ) ) )
			{
				const CShutdownInterfaceAcknowledgement *ack_message = static_cast< const CShutdownInterfaceAcknowledgement * >( raw_message );

				ASSERT_TRUE( ack_message->Get_Shutdown_Key() == UI_KEY );
			}
			else
			{
				ASSERT_TRUE( Loki::TypeInfo( typeid( *raw_message ) ) == Loki::TypeInfo( typeid( CShutdownThreadAcknowledgement ) ) );
			}
		}
	}
}

TEST_F( ThreadTaskTests, Shutdown_Hard )
{
	static const std::wstring LOG_MESSAGE( L"Hard shutdown" );

	SThreadKey test_key( TS_AI, 1, 1  );
	CThreadTaskBaseTester thread_tester( new CTestThreadTask( test_key ) );

	shared_ptr< CThreadConnection > ui_conn( new CThreadConnection( UI_KEY ) );
	shared_ptr< CThreadConnection > log_conn( new CThreadConnection( LOG_THREAD_KEY ) );

	shared_ptr< CThreadMessageFrame > added_frame( new CThreadMessageFrame( MANAGER_THREAD_KEY ) );

	added_frame->Add_Message( shared_ptr< const IThreadMessage >( new CAddInterfaceMessage( UI_KEY, ui_conn->Get_Writer_Interface() ) ) );
	added_frame->Add_Message( shared_ptr< const IThreadMessage >( new CAddInterfaceMessage( LOG_THREAD_KEY, log_conn->Get_Writer_Interface() ) ) );
	added_frame->Add_Message( shared_ptr< const IThreadMessage >( new CShutdownInterfaceMessage( UI_KEY ) ) );
	added_frame->Add_Message( shared_ptr< const IThreadMessage >( new CShutdownThreadRequest( true ) ) );

	thread_tester.Get_Self_Proxy()->Get_Writer_Interface()->Add_Frame( added_frame );

	// message going nowhere
	shared_ptr< CScheduledTask > db_task( new CSendDBMessageTask( 0.0 ) );
	thread_tester.Get_Thread_Task()->Get_Task_Scheduler()->Submit_Task( db_task );

	// message to ui thread, but shouldn't get sent since it's a hard shutdown
	shared_ptr< CScheduledTask > ui_task( new CSendUIMessageTask( 0.0 ) );
	thread_tester.Get_Thread_Task()->Get_Task_Scheduler()->Submit_Task( ui_task );

	// log message
	thread_tester.Log( LOG_MESSAGE );

	thread_tester.Service( 0.0 );

	// thread should have no outbound frames
	auto outbound_frames = thread_tester.Get_Frame_Table();
	ASSERT_TRUE( outbound_frames.size() == 0 );

	// thread should have no interfaces
	auto interfaces = thread_tester.Get_Interface_Table();
	ASSERT_TRUE( interfaces.size() == 0 );	

	// verify a single log message sent
	std::vector< shared_ptr< CThreadMessageFrame > > frames;
	log_conn->Get_Reader_Interface()->Remove_Frames( frames );

	ASSERT_TRUE( frames.size() == 1 );

	auto frame = frames[ 0 ];
	for ( auto iter = frame->Get_Frame_Begin(); iter != frame->Get_Frame_End(); ++iter )
	{
		const IThreadMessage *raw_message = iter->get();

		ASSERT_TRUE( Loki::TypeInfo( typeid( *raw_message ) ) == Loki::TypeInfo( typeid( CLogRequestMessage ) ) );

		const CLogRequestMessage *log_message = static_cast< const CLogRequestMessage * >( raw_message );

		ASSERT_TRUE( log_message->Get_Source_Key() == test_key );
		ASSERT_TRUE( log_message->Get_Message() == LOG_MESSAGE );
	}

	// verify the ui message was not sent
	ui_conn->Get_Reader_Interface()->Remove_Frames( frames );
	ASSERT_TRUE( frames.size() == 0 );

	// verify the acknowledgement messages sent to the manager
	thread_tester.Get_Manager_Proxy()->Get_Reader_Interface()->Remove_Frames( frames );

	for ( auto outer_iter = frames.cbegin(); outer_iter != frames.cend(); ++outer_iter )
	{
		frame = *outer_iter;
		ASSERT_TRUE( frame->Get_Key() == test_key );

		for ( auto iter = frame->Get_Frame_Begin(); iter != frame->Get_Frame_End(); ++iter )
		{
			const IThreadMessage *raw_message = iter->get();

			if ( Loki::TypeInfo( typeid( *raw_message ) ) == Loki::TypeInfo( typeid( CShutdownInterfaceAcknowledgement ) ) )
			{
				const CShutdownInterfaceAcknowledgement *ack_message = static_cast< const CShutdownInterfaceAcknowledgement * >( raw_message );

				ASSERT_TRUE( ack_message->Get_Shutdown_Key() == UI_KEY );
			}
			else
			{
				ASSERT_TRUE( Loki::TypeInfo( typeid( *raw_message ) ) == Loki::TypeInfo( typeid( CShutdownThreadAcknowledgement ) ) );
			}
		}
	}

}