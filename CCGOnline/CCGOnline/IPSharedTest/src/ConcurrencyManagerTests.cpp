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
#include <IPShared/Concurrency/ConcurrencyManager.h>
#include <IPShared/Concurrency/MailboxInterfaces.h>
#include <IPShared/Concurrency/Messaging/ProcessManagementMessages.h>
#include <IPShared/Concurrency/Messaging/ExchangeMailboxMessages.h>
#include <IPShared/Concurrency/ProcessConstants.h>
#include <IPShared/Concurrency/ProcessMessageFrame.h>
#include <IPShared/Concurrency/ProcessStatics.h>
#include <IPShared/Concurrency/TaskProcessBase.h>
#include <IPShared/Logging/LogInterface.h>
#include <IPShared/TaskScheduler/ScheduledTask.h>
#include <IPShared/TaskScheduler/TaskScheduler.h>
#include <IPShared/Time/TimeKeeper.h>
#include <IPSharedTest/SharedTestProcessSubject.h>
#include <gtest/gtest.h>
#include <loki/LokiTypeInfo.h>

#include <thread>

using namespace IP::Execution;
using namespace IP::Execution::Messaging;
using namespace IP::Time;

class ConcurrencyManagerTests : public testing::Test 
{
	protected:  

};

class CTimeKeeperProxy : public CTimeKeeper
{
	public:

		using BASECLASS = CTimeKeeper;

		CTimeKeeperProxy( void ) :
			BASECLASS(),
			CurrentTime()
		{}

		virtual ~CTimeKeeperProxy() {}

		virtual SystemTimePoint Get_Current_Time( void ) const override { return CurrentTime; }

		virtual void Set_Base_Time( SystemTimePoint base_time ) override {
			BASECLASS::Set_Base_Time( base_time );
			CurrentTime = base_time; 
		}

	   void Advance_Current_Time( double seconds ) {
			uint64_t advance_microseconds = static_cast< uint64_t >( seconds * std::chrono::microseconds::period::num / std::chrono::microseconds::period::den );
			std::chrono::microseconds advance_duration( advance_microseconds );

			CurrentTime += std::chrono::duration_cast< SystemDuration >( advance_duration );
		}
		
	private:

		SystemTimePoint CurrentTime;

};

class CProcessBaseExaminer
{
	public:

		CProcessBaseExaminer( const std::shared_ptr< CProcessBase > &virtual_process ) :
			Process( virtual_process )
		{}

		const CProcessBase::FrameTableType &Get_Frame_Table( void ) const { return Process->PendingOutboundFrames; }
		const CProcessBase::MailboxTableType &Get_Mailbox_Table( void ) const { return Process->Mailboxes; }
		std::shared_ptr< CReadOnlyMailbox > Get_My_Mailbox( void ) const { return Process->MyMailbox; }

		std::shared_ptr< CWriteOnlyMailbox > Get_Logging_Mailbox( void ) const { return Process->LoggingMailbox; }
		std::shared_ptr< CWriteOnlyMailbox > Get_Manager_Mailbox( void ) const { return Process->ManagerMailbox; }

		bool Has_Mailbox_With_Properties( const SProcessProperties &properties ) const
		{
			auto mailboxes = Get_Mailbox_Table();
			for ( auto iter = mailboxes.cbegin(), end = mailboxes.cend(); iter != end; ++iter )
			{
				if ( properties.Matches( iter->second->Get_Properties() ) )
				{
					return true;
				}
			}

			return false;
		}

		bool Has_Mailbox( EProcessID process_id ) const
		{
			return Process->Get_Mailbox( process_id ) != nullptr;
		}

	private:

		std::shared_ptr< CProcessBase > Process;
};

class CConcurrencyManagerTester
{
	public:

		CConcurrencyManagerTester( void ) :
			Manager( IP::Make_Unique< CConcurrencyManager >( MEMORY_TAG ) )
		{
			auto timekeeper = IP::New< CTimeKeeperProxy >( MEMORY_TAG );
			timekeeper->Set_Base_Time( Get_Current_System_Time() );

			Manager->TimeKeeper.reset( timekeeper );
		}

		~CConcurrencyManagerTester()
		{
		}

		void Setup_For_Run( const std::shared_ptr< IManagedProcess > &virtual_process )
		{
			Manager->Initialize( false );
			Manager->Setup_For_Run( virtual_process );
		}

		void Run_One_Iteration( void )
		{
			Manager->Service_One_Iteration();

			Advance_Current_Time( .1 );

			// Wait for all reschedules to return
			while ( !Are_Rescheduled_Processes_Finished() )
			{
				std::this_thread::sleep_for( std::chrono::milliseconds( 0 ) );
			}
		}

		void Add_Rescheduled_Process( EProcessID process_id ) { RescheduledProcesses.insert( process_id ); }
		void Remove_Rescheduled_Process( EProcessID process_id ) { RescheduledProcesses.erase( process_id ); }

		bool Are_Rescheduled_Processes_Finished( void )
		{
			auto thread_set_copy = RescheduledProcesses;

			IP::Vector< IP::UniquePtr< CProcessMessageFrame > > frames;
			std::shared_ptr< CReadOnlyMailbox > read_interface = Manager->Get_My_Mailbox();
			read_interface->Remove_Frames( frames );

			// go through all frames, looking for reschedule messages
			for ( uint32_t i = 0; i < frames.size(); ++i )
			{
				IP::UniquePtr< CProcessMessageFrame > &frame = frames[ i ];
				EProcessID process_id = frame->Get_Process_ID();

				for ( auto iter = frame->cbegin(), end = frame->cend(); iter != end; ++iter )
				{
					const IProcessMessage *raw_message = iter->get();
					if ( Loki::TypeInfo( typeid( *raw_message ) ) == Loki::TypeInfo( typeid( CRescheduleProcessMessage ) ) )
					{
						if ( thread_set_copy.find( process_id ) != thread_set_copy.end() )
						{
							thread_set_copy.erase( process_id );
						}
					}
				}
			}

			// restore all the frames in the manager mailbox
			std::shared_ptr< CWriteOnlyMailbox > write_interface = Manager->Get_Mailbox( MANAGER_PROCESS_ID );
			for ( uint32_t i = 0; i < frames.size(); ++i )
			{
				write_interface->Add_Frame( frames[ i ] );
			}

			return thread_set_copy.size() == 0;
		}

		bool Has_Process( EProcessID process_id ) const { return Manager->Get_Record( process_id ) != nullptr; }
		bool Has_Process_With_Properties( const SProcessProperties &properties ) const {
			return Get_Virtual_Process_By_Property_Match( properties ) != nullptr;
		}
		
		std::shared_ptr< IManagedProcess > Get_Virtual_Process( EProcessID process_id ) const { return Manager->Get_Process( process_id ); }

		std::shared_ptr< IManagedProcess > Get_Virtual_Process_By_Property_Match( const SProcessProperties &properties ) const { 
			IP::Vector< std::shared_ptr< IManagedProcess > > processes;
			Manager->Enumerate_Processes( processes );
			for ( uint32_t i = 0; i < processes.size(); ++i )
			{
				if ( properties.Matches( processes[ i ]->Get_Properties() ) )
				{
					return processes[ i ];
				}
			}

			return nullptr;
		}

		void Shutdown( void )
		{
			if ( Manager->ProcessRecords.size() > 0 )
			{
				auto frame = IP::Make_Unique< CProcessMessageFrame >( MEMORY_TAG, MANAGER_PROCESS_ID );
				auto shutdown_msg = IP::Make_Const_Process_Message< CShutdownManagerMessage >( MEMORY_TAG );
				frame->Add_Message( shutdown_msg );

				std::shared_ptr< CWriteOnlyMailbox > write_interface = Manager->Get_Mailbox( MANAGER_PROCESS_ID );
				write_interface->Add_Frame( frame );

				Wait_For_Shutdown();
			}
		}

		void Wait_For_Shutdown( void )
		{
			while ( Manager->ProcessRecords.size() > 0 )
			{
				Advance_Current_Time( .1 );
				Manager->Service_One_Iteration();

				std::this_thread::sleep_for( std::chrono::milliseconds( 0 ) );
			}

			RescheduledProcesses.clear();
		}

		const CConcurrencyManager::FrameTableType &Get_Frame_Table( void ) const { return Manager->PendingOutboundFrames; }

		std::shared_ptr< CWriteOnlyMailbox > Get_Manager_Mailbox( void ) const { return Manager->Get_Mailbox( MANAGER_PROCESS_ID ); }

	private:

		void Advance_Current_Time( double seconds )
		{
			CTimeKeeperProxy *timekeeper = static_cast< CTimeKeeperProxy * >( Manager->TimeKeeper.get() );
			timekeeper->Advance_Current_Time( seconds );
		}

		IP::UniquePtr< CConcurrencyManager > Manager;

		std::set< EProcessID > RescheduledProcesses;
};

class CDoNothingProcess : public CTaskProcessBase
{
	public:

		using BASECLASS = CTaskProcessBase;

		CDoNothingProcess( const SProcessProperties &properties ) :
			BASECLASS( properties ),
			ServiceCount( 0 )
		{}

		virtual bool Is_Root_Thread( void ) const { return true; }

		virtual void Run( const CProcessExecutionContext &context )
		{
			BASECLASS::Run( context );

			ServiceCount++;
		}

		uint32_t Get_Service_Count( void ) const { return ServiceCount; }

	private:

		uint32_t ServiceCount;
};

static const EProcessID AI_PROCESS_ID( EProcessID::FIRST_FREE_ID );
static const SProcessProperties AI_PROPS( ETestExtendedProcessSubject::AI );

TEST_F( ConcurrencyManagerTests, Setup )
{
	CConcurrencyManagerTester manager_tester;

	manager_tester.Setup_For_Run( IP::Make_Shared_Upcast< CDoNothingProcess, IManagedProcess >( MEMORY_TAG, AI_PROPS ) );

	manager_tester.Add_Rescheduled_Process( AI_PROCESS_ID );

	// Verify setup state
	// 3 thread records: manager, log, test_key
	ASSERT_TRUE( manager_tester.Has_Process( AI_PROCESS_ID ) );
	ASSERT_TRUE( manager_tester.Has_Process( MANAGER_PROCESS_ID ) );
	ASSERT_TRUE( manager_tester.Has_Process( LOGGING_PROCESS_ID ) );

	// process should have a log interface
	CProcessBaseExaminer test_examiner( std::static_pointer_cast< CProcessBase >( manager_tester.Get_Virtual_Process( AI_PROCESS_ID ) ) );
	auto mailboxes = test_examiner.Get_Mailbox_Table();
	ASSERT_TRUE( mailboxes.size() == 0 );
	ASSERT_TRUE( test_examiner.Get_Logging_Mailbox().get() != nullptr );

	manager_tester.Shutdown();
}



static const SProcessProperties VP_PROPERTY1( ETestExtendedProcessSubject::AI, 1, 1, 1 );
static const SProcessProperties VP_PROPERTY2( ETestExtendedProcessSubject::AI, 1, 2, 1 );
static const SProcessProperties VP_PROPERTY3( ETestExtendedProcessSubject::AI, 1, 3, 4 );
static const SProcessProperties VP_PROPERTY4( ETestExtendedProcessSubject::AI, 2, 1, 2 );
static const SProcessProperties VP_PROPERTY5( ETestExtendedProcessSubject::AI, 1, 1, 3 );

class CMailboxTestProcess : public CDoNothingProcess
{
	public:

		using BASECLASS = CDoNothingProcess;

		CMailboxTestProcess( const SProcessProperties &properties ) :
			BASECLASS( properties ),
			HasBeenServiced( false )
		{}

		virtual bool Is_Root_Thread( void ) const { return true; }

		virtual void Run( const CProcessExecutionContext &context )
		{
			if ( !HasBeenServiced )
			{
				HasBeenServiced = true;

				const SProcessProperties &properties = Get_Properties();
				if ( properties == VP_PROPERTY1 )
				{
					auto get_mailbox_msg = IP::Make_Process_Message< CGetMailboxByPropertiesRequest >( MEMORY_TAG, VP_PROPERTY2 );
					Send_Manager_Message( get_mailbox_msg );
				}
				else if ( properties == VP_PROPERTY2 )
				{
					SProcessProperties multimatch( ETestExtendedProcessSubject::AI, 1, 0, 0 );
					auto get_mailbox_msg = IP::Make_Process_Message< CGetMailboxByPropertiesRequest >( MEMORY_TAG, multimatch );
					Send_Manager_Message( get_mailbox_msg );
				}
				else if ( properties == VP_PROPERTY3 )
				{
					SProcessProperties multimatch( ETestExtendedProcessSubject::AI, 2, 0, 0 );
					auto get_mailbox_msg = IP::Make_Process_Message< CGetMailboxByPropertiesRequest >( MEMORY_TAG, multimatch );
					Send_Manager_Message( get_mailbox_msg );
				}
				else if ( properties == VP_PROPERTY4 )
				{
					auto get_mailbox_msg = IP::Make_Process_Message< CGetMailboxByIDRequest >( MEMORY_TAG, EProcessID::FIRST_FREE_ID );
					Send_Manager_Message( get_mailbox_msg );
				}
				else if ( properties == VP_PROPERTY5 )
				{
					auto get_mailbox_msg = IP::Make_Process_Message< CGetMailboxByIDRequest >( MEMORY_TAG, static_cast< EProcessID >( static_cast< uint64_t >( EProcessID::FIRST_FREE_ID ) + 50 ) );
					Send_Manager_Message( get_mailbox_msg );
				}
			}

			BASECLASS::Run( context );
		}

	private:

		bool HasBeenServiced;
};

class CSpawnMailboxGetProcess : public CTaskProcessBase
{
	public:

		using BASECLASS = CTaskProcessBase;

		CSpawnMailboxGetProcess( const SProcessProperties &properties ) :
			BASECLASS( properties ),
			HasBeenServiced( false )
		{}

		virtual bool Is_Root_Thread( void ) const { return true; }

		virtual void Run( const CProcessExecutionContext &context )
		{
			if ( !HasBeenServiced )
			{
				auto add_message1 = IP::Make_Process_Message< CAddNewProcessMessage >( MEMORY_TAG, IP::Make_Process< CMailboxTestProcess >( MEMORY_TAG, VP_PROPERTY1 ), true, false );
				Send_Manager_Message( add_message1 );

				auto add_message2 = IP::Make_Process_Message< CAddNewProcessMessage >( MEMORY_TAG, IP::Make_Process< CMailboxTestProcess >( MEMORY_TAG, VP_PROPERTY2 ), false, true );
				Send_Manager_Message( add_message2 );
				
				auto add_message3 = IP::Make_Process_Message< CAddNewProcessMessage >( MEMORY_TAG, IP::Make_Process< CMailboxTestProcess >( MEMORY_TAG, VP_PROPERTY3 ), false, true );
				Send_Manager_Message( add_message3 );
				
				auto add_message4 = IP::Make_Process_Message< CAddNewProcessMessage >( MEMORY_TAG, IP::Make_Process< CMailboxTestProcess >( MEMORY_TAG, VP_PROPERTY4 ), false, false );
				Send_Manager_Message( add_message4 );
				
				auto add_message5 = IP::Make_Process_Message< CAddNewProcessMessage >( MEMORY_TAG, IP::Make_Process< CMailboxTestProcess >( MEMORY_TAG, VP_PROPERTY5 ), false, false );
				Send_Manager_Message( add_message5 );

				HasBeenServiced = true;
			}

			BASECLASS::Run( context );
		}

	private:

		bool HasBeenServiced;
};

static const SProcessProperties SPAWN_PROCESS_PROPERTIES( ETestExtendedProcessSubject::DATABASE, 1, 1, 1 );

void Verify_Interfaces_Present( const CConcurrencyManagerTester &manager_tester )
{
	CProcessBaseExaminer spawn_process( std::static_pointer_cast< CProcessBase >( manager_tester.Get_Virtual_Process_By_Property_Match( SPAWN_PROCESS_PROPERTIES ) ) );
	auto mailboxes = spawn_process.Get_Mailbox_Table();
	ASSERT_TRUE( mailboxes.size() == 1 );
	ASSERT_TRUE( spawn_process.Has_Mailbox_With_Properties( VP_PROPERTY1 ) );
	ASSERT_TRUE( spawn_process.Get_Logging_Mailbox().get() != nullptr );
	ASSERT_TRUE( spawn_process.Get_Manager_Mailbox().get() != nullptr );

	CProcessBaseExaminer test_process1( std::static_pointer_cast< CProcessBase >( manager_tester.Get_Virtual_Process_By_Property_Match( VP_PROPERTY1 ) ) );
	mailboxes = test_process1.Get_Mailbox_Table();
	ASSERT_TRUE( mailboxes.size() == 1 );
	ASSERT_TRUE( test_process1.Has_Mailbox_With_Properties( VP_PROPERTY2 ) );
	ASSERT_TRUE( test_process1.Get_Logging_Mailbox().get() != nullptr );
	ASSERT_TRUE( test_process1.Get_Manager_Mailbox().get() != nullptr );

	CProcessBaseExaminer test_process2( std::static_pointer_cast< CProcessBase >( manager_tester.Get_Virtual_Process_By_Property_Match( VP_PROPERTY2 ) ) );
	mailboxes = test_process2.Get_Mailbox_Table();
	ASSERT_TRUE( mailboxes.size() == 4 );
	ASSERT_TRUE( test_process2.Has_Mailbox_With_Properties( SPAWN_PROCESS_PROPERTIES ) );
	ASSERT_TRUE( test_process2.Has_Mailbox_With_Properties( VP_PROPERTY1 ) );
	ASSERT_TRUE( test_process2.Has_Mailbox_With_Properties( VP_PROPERTY3 ) );
	ASSERT_TRUE( test_process2.Has_Mailbox_With_Properties( VP_PROPERTY5 ) );
	ASSERT_TRUE( test_process2.Get_Logging_Mailbox().get() != nullptr );
	ASSERT_TRUE( test_process2.Get_Manager_Mailbox().get() != nullptr );

	CProcessBaseExaminer test_process3( std::static_pointer_cast< CProcessBase >( manager_tester.Get_Virtual_Process_By_Property_Match( VP_PROPERTY3 ) ) );
	mailboxes = test_process3.Get_Mailbox_Table();
	ASSERT_TRUE( mailboxes.size() == 2 );
	ASSERT_TRUE( test_process3.Has_Mailbox_With_Properties( SPAWN_PROCESS_PROPERTIES ) );
	ASSERT_TRUE( test_process3.Has_Mailbox_With_Properties( VP_PROPERTY4 ) );
	ASSERT_TRUE( test_process3.Get_Logging_Mailbox().get() != nullptr );
	ASSERT_TRUE( test_process3.Get_Manager_Mailbox().get() != nullptr );

	CProcessBaseExaminer test_process4( std::static_pointer_cast< CProcessBase >( manager_tester.Get_Virtual_Process_By_Property_Match( VP_PROPERTY4 ) ) );
	mailboxes = test_process4.Get_Mailbox_Table();
	ASSERT_TRUE( mailboxes.size() == 1 );
	ASSERT_TRUE( test_process4.Has_Mailbox( EProcessID::FIRST_FREE_ID ) );
	ASSERT_TRUE( test_process4.Get_Logging_Mailbox().get() != nullptr );
	ASSERT_TRUE( test_process4.Get_Manager_Mailbox().get() != nullptr );

	CProcessBaseExaminer test_process5( std::static_pointer_cast< CProcessBase >( manager_tester.Get_Virtual_Process_By_Property_Match( VP_PROPERTY5 ) ) );
	mailboxes = test_process5.Get_Mailbox_Table();
	ASSERT_TRUE( mailboxes.size() == 0 );
	ASSERT_TRUE( test_process5.Get_Logging_Mailbox().get() != nullptr );
	ASSERT_TRUE( test_process5.Get_Manager_Mailbox().get() != nullptr );
}

TEST_F( ConcurrencyManagerTests, Interface_Get1 )
{
	CConcurrencyManagerTester manager_tester;

	manager_tester.Setup_For_Run( IP::Make_Shared_Upcast< CSpawnMailboxGetProcess, IManagedProcess >( MEMORY_TAG, SPAWN_PROCESS_PROPERTIES ) );

	EProcessID spawn_id = manager_tester.Get_Virtual_Process_By_Property_Match( SPAWN_PROCESS_PROPERTIES )->Get_ID();
	manager_tester.Add_Rescheduled_Process( spawn_id );
	manager_tester.Run_One_Iteration();
	manager_tester.Run_One_Iteration();

	ASSERT_TRUE( manager_tester.Has_Process( spawn_id ) );
	ASSERT_TRUE( manager_tester.Has_Process_With_Properties( VP_PROPERTY1 ) );
	ASSERT_TRUE( manager_tester.Has_Process_With_Properties( VP_PROPERTY2 ) );
	ASSERT_TRUE( manager_tester.Has_Process_With_Properties( VP_PROPERTY3 ) );
	ASSERT_TRUE( manager_tester.Has_Process_With_Properties( VP_PROPERTY4 ) );
	ASSERT_TRUE( manager_tester.Has_Process_With_Properties( VP_PROPERTY5 ) );

	EProcessID vp_1 = manager_tester.Get_Virtual_Process_By_Property_Match( VP_PROPERTY1 )->Get_ID();
	EProcessID vp_2 = manager_tester.Get_Virtual_Process_By_Property_Match( VP_PROPERTY2 )->Get_ID();
	EProcessID vp_3 = manager_tester.Get_Virtual_Process_By_Property_Match( VP_PROPERTY3 )->Get_ID();
	EProcessID vp_4 = manager_tester.Get_Virtual_Process_By_Property_Match( VP_PROPERTY4 )->Get_ID();
	EProcessID vp_5 = manager_tester.Get_Virtual_Process_By_Property_Match( VP_PROPERTY5 )->Get_ID();
	ASSERT_TRUE( manager_tester.Has_Process( vp_1 ) );
	ASSERT_TRUE( manager_tester.Has_Process( vp_2 ) );
	ASSERT_TRUE( manager_tester.Has_Process( vp_3 ) );
	ASSERT_TRUE( manager_tester.Has_Process( vp_4 ) );
	ASSERT_TRUE( manager_tester.Has_Process( vp_5 ) );

	manager_tester.Add_Rescheduled_Process( vp_1 );
	manager_tester.Add_Rescheduled_Process( vp_2 );
	manager_tester.Add_Rescheduled_Process( vp_3 );
	manager_tester.Add_Rescheduled_Process( vp_4 );
	manager_tester.Add_Rescheduled_Process( vp_5 );
	manager_tester.Run_One_Iteration();

	manager_tester.Run_One_Iteration();

	Verify_Interfaces_Present( manager_tester );

	manager_tester.Shutdown();
}

class CSuicidalProcess : public CTaskProcessBase
{
	public:

		using BASECLASS = CTaskProcessBase;

		CSuicidalProcess( const SProcessProperties &properties ) :
			BASECLASS( properties ),
			HasBeenServiced( false )
		{}

		virtual bool Is_Root_Thread( void ) const { return true; }

		virtual void Run( const CProcessExecutionContext &context )
		{
			if ( !HasBeenServiced )
			{
				auto shutdown_msg = IP::Make_Process_Message< CShutdownProcessMessage >( MEMORY_TAG, Get_ID() );
				Send_Manager_Message( shutdown_msg );
				HasBeenServiced = true;
			}

			BASECLASS::Run( context );
		}

	private:

		bool HasBeenServiced;
};

static const SProcessProperties SUICIDAL_PROCESS_PROPERTIES( ETestExtendedProcessSubject::DATABASE, 1, 1, 1 );

TEST_F( ConcurrencyManagerTests, Run_Once_And_Shutdown_Self )
{
	CConcurrencyManagerTester manager_tester;

	manager_tester.Setup_For_Run( IP::Make_Shared_Upcast< CSuicidalProcess, IManagedProcess >( MEMORY_TAG, SUICIDAL_PROCESS_PROPERTIES ) );

	manager_tester.Wait_For_Shutdown();
}

TEST_F( ConcurrencyManagerTests, Stress )
{
	for ( uint32_t i = 0; i < 50; ++i )
	{
		CConcurrencyManagerTester manager_tester;

		manager_tester.Setup_For_Run( IP::Make_Shared_Upcast< CSuicidalProcess, IManagedProcess >( MEMORY_TAG, SUICIDAL_PROCESS_PROPERTIES ) );

		manager_tester.Wait_For_Shutdown();
	}
}
