/**********************************************************************************************************************

	ConcurrencyManagerTests.cpp
		defines unit tests for concurrency manager functionality

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

#include "Concurrency/ConcurrencyManager.h"
#include "Concurrency/MailboxInterfaces.h"
#include "Concurrency/ProcessStatics.h"
#include "Concurrency/TaskProcessBase.h"
#include "Concurrency/ProcessSubject.h"
#include "Concurrency/ProcessConstants.h"
#include "Concurrency/ProcessMessageFrame.h"
#include "Concurrency/Messaging/ProcessManagementMessages.h"
#include "Concurrency/Messaging/ExchangeMailboxMessages.h"
#include "Logging/LogInterface.h"
#include "Time/TimeType.h"
#include "TaskScheduler/ScheduledTask.h"
#include "TaskScheduler/TaskScheduler.h"
#include "PlatformProcess.h"

class ConcurrencyManagerTests : public testing::Test 
{
	protected:  

};

class CProcessBaseExaminer
{
	public:

		CProcessBaseExaminer( const shared_ptr< CProcessBase > &virtual_process ) :
			Process( virtual_process )
		{}

		const CProcessBase::FrameTableType &Get_Frame_Table( void ) const { return Process->PendingOutboundFrames; }
		const CProcessBase::MailboxTableType &Get_Mailbox_Table( void ) const { return Process->Mailboxes; }
		shared_ptr< CReadOnlyMailbox > Get_My_Mailbox( void ) const { return Process->MyMailbox; }

		shared_ptr< CWriteOnlyMailbox > Get_Logging_Mailbox( void ) const { return Process->LoggingMailbox; }
		shared_ptr< CWriteOnlyMailbox > Get_Manager_Mailbox( void ) const { return Process->ManagerMailbox; }

		bool Has_Mailbox_With_Properties( const SProcessProperties &properties ) const
		{
			auto mailboxes = Get_Mailbox_Table();
			for ( auto iter = mailboxes.cbegin(); iter != mailboxes.cend(); ++iter )
			{
				if ( properties.Matches( iter->second->Get_Properties() ) )
				{
					return true;
				}
			}

			return false;
		}

		bool Has_Mailbox( EProcessID::Enum process_id ) const
		{
			return Process->Get_Mailbox( process_id ) != nullptr;
		}

	private:

		shared_ptr< CProcessBase > Process;
};

class CConcurrencyManagerTester
{
	public:

		CConcurrencyManagerTester( void ) :
			Manager( new CConcurrencyManager )
		{
		}

		~CConcurrencyManagerTester()
		{
		}

		void Setup_For_Run( const shared_ptr< IManagedProcess > &virtual_process )
		{
			Manager->Initialize( false );
			Manager->Setup_For_Run( virtual_process );
		}

		void Run_One_Iteration( void )
		{
			Manager->Service_One_Iteration();
			Manager->Set_Game_Time( Manager->Get_Game_Time() + .1 );

			// Wait for all reschedules to return
			while ( !Are_Rescheduled_Processes_Finished() )
			{
				NPlatform::Sleep( 0 );
			}
		}

		void Add_Rescheduled_Process( EProcessID::Enum process_id ) { RescheduledProcesses.insert( process_id ); }
		void Remove_Rescheduled_Process( EProcessID::Enum process_id ) { RescheduledProcesses.erase( process_id ); }

		bool Are_Rescheduled_Processes_Finished( void )
		{
			auto thread_set_copy = RescheduledProcesses;

			std::vector< shared_ptr< CProcessMessageFrame > > frames;
			shared_ptr< CReadOnlyMailbox > read_interface = Manager->Get_My_Mailbox();
			read_interface->Remove_Frames( frames );

			// go through all frames, looking for reschedule messages
			for ( uint32 i = 0; i < frames.size(); ++i )
			{
				shared_ptr< CProcessMessageFrame > frame = frames[ i ];
				EProcessID::Enum process_id = frame->Get_Process_ID();
				for ( auto iter = frame->Get_Frame_Begin(); iter != frame->Get_Frame_End(); ++iter )
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
			shared_ptr< CWriteOnlyMailbox > write_interface = Manager->Get_Mailbox( MANAGER_PROCESS_ID );
			for ( uint32 i = 0; i < frames.size(); ++i )
			{
				write_interface->Add_Frame( frames[ i ] );
			}

			return thread_set_copy.size() == 0;
		}

		bool Has_Process( EProcessID::Enum process_id ) const { return Manager->Get_Record( process_id ) != nullptr; }
		bool Has_Process_With_Properties( const SProcessProperties &properties ) const {
			return Get_Virtual_Process_By_Property_Match( properties ) != nullptr;
		}
		
		shared_ptr< IManagedProcess > Get_Virtual_Process( EProcessID::Enum process_id ) const { return Manager->Get_Process( process_id ); }

		shared_ptr< IManagedProcess > Get_Virtual_Process_By_Property_Match( const SProcessProperties &properties ) const { 
			std::vector< shared_ptr< IManagedProcess > > processes;
			Manager->Enumerate_Processes( processes );
			for ( uint32 i = 0; i < processes.size(); ++i )
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
				shared_ptr< CProcessMessageFrame > frame( new CProcessMessageFrame( MANAGER_PROCESS_ID ) );
				frame->Add_Message( shared_ptr< const IProcessMessage >( new CShutdownManagerMessage ) );

				shared_ptr< CWriteOnlyMailbox > write_interface = Manager->Get_Mailbox( MANAGER_PROCESS_ID );
				write_interface->Add_Frame( frame );

				Wait_For_Shutdown();
			}
		}

		void Wait_For_Shutdown( void )
		{
			while ( Manager->ProcessRecords.size() > 0 )
			{
				Manager->Set_Game_Time( Manager->Get_Game_Time() + .1 );
				Manager->Service_One_Iteration();

				NPlatform::Sleep( 0 );
			}

			RescheduledProcesses.clear();
		}

		const CConcurrencyManager::FrameTableType &Get_Frame_Table( void ) const { return Manager->PendingOutboundFrames; }

		shared_ptr< CWriteOnlyMailbox > Get_Manager_Mailbox( void ) const { return Manager->Get_Mailbox( MANAGER_PROCESS_ID ); }

	private:

		unique_ptr< CConcurrencyManager > Manager;

		std::set< EProcessID::Enum > RescheduledProcesses;
};

class CDoNothingProcess : public CTaskProcessBase
{
	public:

		typedef CTaskProcessBase BASECLASS;

		CDoNothingProcess( const SProcessProperties &properties ) :
			BASECLASS( properties ),
			ServiceCount( 0 )
		{}

		virtual ETimeType Get_Time_Type( void ) const { return TT_GAME_TIME; }
		virtual bool Is_Root_Thread( void ) const { return true; }

		virtual void Run( const CProcessExecutionContext &context )
		{
			BASECLASS::Run( context );

			ServiceCount++;
		}

		uint32 Get_Service_Count( void ) const { return ServiceCount; }

	private:

		uint32 ServiceCount;
};

namespace EManagerTestProcessSubject
{
	enum Enum
	{
		AI = EProcessSubject::NEXT_FREE_VALUE,
		DATABASE,
		UI
	};
}

static const EProcessID::Enum AI_PROCESS_ID( EProcessID::FIRST_FREE_ID );
static const SProcessProperties AI_PROPS( EManagerTestProcessSubject::AI );

TEST_F( ConcurrencyManagerTests, Setup )
{
	CConcurrencyManagerTester manager_tester;

	manager_tester.Setup_For_Run( shared_ptr< IManagedProcess >( new CDoNothingProcess( AI_PROPS ) ) );

	manager_tester.Add_Rescheduled_Process( AI_PROCESS_ID );

	// Verify setup state
	// 3 thread records: manager, log, test_key
	ASSERT_TRUE( manager_tester.Has_Process( AI_PROCESS_ID ) );
	ASSERT_TRUE( manager_tester.Has_Process( MANAGER_PROCESS_ID ) );
	ASSERT_TRUE( manager_tester.Has_Process( LOGGING_PROCESS_ID ) );

	// process should have a log interface
	CProcessBaseExaminer test_examiner( static_pointer_cast< CProcessBase >( manager_tester.Get_Virtual_Process( AI_PROCESS_ID ) ) );
	auto mailboxes = test_examiner.Get_Mailbox_Table();
	ASSERT_TRUE( mailboxes.size() == 0 );
	ASSERT_TRUE( test_examiner.Get_Logging_Mailbox().get() != nullptr );

	manager_tester.Shutdown();
}



static const SProcessProperties VP_PROPERTY1( EManagerTestProcessSubject::AI, 1, 1, 1 );
static const SProcessProperties VP_PROPERTY2( EManagerTestProcessSubject::AI, 1, 2, 1 );
static const SProcessProperties VP_PROPERTY3( EManagerTestProcessSubject::AI, 1, 3, 4 );
static const SProcessProperties VP_PROPERTY4( EManagerTestProcessSubject::AI, 2, 1, 2 );
static const SProcessProperties VP_PROPERTY5( EManagerTestProcessSubject::AI, 1, 1, 3 );

class CMailboxTestProcess : public CDoNothingProcess
{
	public:

		typedef CDoNothingProcess BASECLASS;

		CMailboxTestProcess( const SProcessProperties &properties ) :
			BASECLASS( properties ),
			HasBeenServiced( false )
		{}

		virtual ETimeType Get_Time_Type( void ) const { return TT_GAME_TIME; }
		virtual bool Is_Root_Thread( void ) const { return true; }

		virtual void Run( const CProcessExecutionContext &context )
		{
			if ( !HasBeenServiced )
			{
				HasBeenServiced = true;

				const SProcessProperties &properties = Get_Properties();
				if ( properties == VP_PROPERTY1 )
				{
					Send_Manager_Message( shared_ptr< const IProcessMessage >( new CGetMailboxByPropertiesRequest( VP_PROPERTY2 ) ) );
				}
				else if ( properties == VP_PROPERTY2 )
				{
					SProcessProperties multimatch( EManagerTestProcessSubject::AI, 1, 0, 0 );
					Send_Manager_Message( shared_ptr< const IProcessMessage >( new CGetMailboxByPropertiesRequest( multimatch ) ) );
				}
				else if ( properties == VP_PROPERTY3 )
				{
					SProcessProperties multimatch( EManagerTestProcessSubject::AI, 2, 0, 0 );
					Send_Manager_Message( shared_ptr< const IProcessMessage >( new CGetMailboxByPropertiesRequest( multimatch ) ) );
				}
				else if ( properties == VP_PROPERTY4 )
				{
					Send_Manager_Message( shared_ptr< const IProcessMessage >( new CGetMailboxByIDRequest( EProcessID::FIRST_FREE_ID ) ) );
				}
				else if ( properties == VP_PROPERTY5 )
				{
					Send_Manager_Message( shared_ptr< const IProcessMessage >( new CGetMailboxByIDRequest( static_cast< EProcessID::Enum >( EProcessID::FIRST_FREE_ID + 50 ) ) ) );
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

		typedef CTaskProcessBase BASECLASS;

		CSpawnMailboxGetProcess( const SProcessProperties &properties ) :
			BASECLASS( properties ),
			HasBeenServiced( false )
		{}

		virtual ETimeType Get_Time_Type( void ) const { return TT_GAME_TIME; }
		virtual bool Is_Root_Thread( void ) const { return true; }

		virtual void Run( const CProcessExecutionContext &context )
		{
			if ( !HasBeenServiced )
			{
				Send_Manager_Message( shared_ptr< const IProcessMessage >( new CAddNewProcessMessage( shared_ptr< IProcess >( new CMailboxTestProcess( VP_PROPERTY1 ) ), true, false ) ) );
				Send_Manager_Message( shared_ptr< const IProcessMessage >( new CAddNewProcessMessage( shared_ptr< IProcess >( new CMailboxTestProcess( VP_PROPERTY2 ) ), false, true ) ) );
				Send_Manager_Message( shared_ptr< const IProcessMessage >( new CAddNewProcessMessage( shared_ptr< IProcess >( new CMailboxTestProcess( VP_PROPERTY3 ) ), false, true ) ) );
				Send_Manager_Message( shared_ptr< const IProcessMessage >( new CAddNewProcessMessage( shared_ptr< IProcess >( new CMailboxTestProcess( VP_PROPERTY4 ) ), false, false ) ) );
				Send_Manager_Message( shared_ptr< const IProcessMessage >( new CAddNewProcessMessage( shared_ptr< IProcess >( new CMailboxTestProcess( VP_PROPERTY5 ) ), false, false ) ) );

				HasBeenServiced = true;
			}

			BASECLASS::Run( context );
		}

	private:

		bool HasBeenServiced;
};

static const SProcessProperties SPAWN_PROCESS_PROPERTIES( EManagerTestProcessSubject::DATABASE, 1, 1, 1 );

void Verify_Interfaces_Present( const CConcurrencyManagerTester &manager_tester )
{
	CProcessBaseExaminer spawn_process( static_pointer_cast< CProcessBase >( manager_tester.Get_Virtual_Process_By_Property_Match( SPAWN_PROCESS_PROPERTIES ) ) );
	auto mailboxes = spawn_process.Get_Mailbox_Table();
	ASSERT_TRUE( mailboxes.size() == 1 );
	ASSERT_TRUE( spawn_process.Has_Mailbox_With_Properties( VP_PROPERTY1 ) );
	ASSERT_TRUE( spawn_process.Get_Logging_Mailbox().get() != nullptr );
	ASSERT_TRUE( spawn_process.Get_Manager_Mailbox().get() != nullptr );

	CProcessBaseExaminer test_process1( static_pointer_cast< CProcessBase >( manager_tester.Get_Virtual_Process_By_Property_Match( VP_PROPERTY1 ) ) );
	mailboxes = test_process1.Get_Mailbox_Table();
	ASSERT_TRUE( mailboxes.size() == 1 );
	ASSERT_TRUE( test_process1.Has_Mailbox_With_Properties( VP_PROPERTY2 ) );
	ASSERT_TRUE( test_process1.Get_Logging_Mailbox().get() != nullptr );
	ASSERT_TRUE( test_process1.Get_Manager_Mailbox().get() != nullptr );

	CProcessBaseExaminer test_process2( static_pointer_cast< CProcessBase >( manager_tester.Get_Virtual_Process_By_Property_Match( VP_PROPERTY2 ) ) );
	mailboxes = test_process2.Get_Mailbox_Table();
	ASSERT_TRUE( mailboxes.size() == 4 );
	ASSERT_TRUE( test_process2.Has_Mailbox_With_Properties( SPAWN_PROCESS_PROPERTIES ) );
	ASSERT_TRUE( test_process2.Has_Mailbox_With_Properties( VP_PROPERTY1 ) );
	ASSERT_TRUE( test_process2.Has_Mailbox_With_Properties( VP_PROPERTY3 ) );
	ASSERT_TRUE( test_process2.Has_Mailbox_With_Properties( VP_PROPERTY5 ) );
	ASSERT_TRUE( test_process2.Get_Logging_Mailbox().get() != nullptr );
	ASSERT_TRUE( test_process2.Get_Manager_Mailbox().get() != nullptr );

	CProcessBaseExaminer test_process3( static_pointer_cast< CProcessBase >( manager_tester.Get_Virtual_Process_By_Property_Match( VP_PROPERTY3 ) ) );
	mailboxes = test_process3.Get_Mailbox_Table();
	ASSERT_TRUE( mailboxes.size() == 2 );
	ASSERT_TRUE( test_process3.Has_Mailbox_With_Properties( SPAWN_PROCESS_PROPERTIES ) );
	ASSERT_TRUE( test_process3.Has_Mailbox_With_Properties( VP_PROPERTY4 ) );
	ASSERT_TRUE( test_process3.Get_Logging_Mailbox().get() != nullptr );
	ASSERT_TRUE( test_process3.Get_Manager_Mailbox().get() != nullptr );

	CProcessBaseExaminer test_process4( static_pointer_cast< CProcessBase >( manager_tester.Get_Virtual_Process_By_Property_Match( VP_PROPERTY4 ) ) );
	mailboxes = test_process4.Get_Mailbox_Table();
	ASSERT_TRUE( mailboxes.size() == 1 );
	ASSERT_TRUE( test_process4.Has_Mailbox( EProcessID::FIRST_FREE_ID ) );
	ASSERT_TRUE( test_process4.Get_Logging_Mailbox().get() != nullptr );
	ASSERT_TRUE( test_process4.Get_Manager_Mailbox().get() != nullptr );

	CProcessBaseExaminer test_process5( static_pointer_cast< CProcessBase >( manager_tester.Get_Virtual_Process_By_Property_Match( VP_PROPERTY5 ) ) );
	mailboxes = test_process5.Get_Mailbox_Table();
	ASSERT_TRUE( mailboxes.size() == 0 );
	ASSERT_TRUE( test_process5.Get_Logging_Mailbox().get() != nullptr );
	ASSERT_TRUE( test_process5.Get_Manager_Mailbox().get() != nullptr );
}

TEST_F( ConcurrencyManagerTests, Interface_Get1 )
{
	CConcurrencyManagerTester manager_tester;

	manager_tester.Setup_For_Run( shared_ptr< IManagedProcess >( new CSpawnMailboxGetProcess( SPAWN_PROCESS_PROPERTIES ) ) );

	EProcessID::Enum spawn_id = manager_tester.Get_Virtual_Process_By_Property_Match( SPAWN_PROCESS_PROPERTIES )->Get_ID();
	manager_tester.Add_Rescheduled_Process( spawn_id );
	manager_tester.Run_One_Iteration();
	manager_tester.Run_One_Iteration();

	ASSERT_TRUE( manager_tester.Has_Process( spawn_id ) );
	ASSERT_TRUE( manager_tester.Has_Process_With_Properties( VP_PROPERTY1 ) );
	ASSERT_TRUE( manager_tester.Has_Process_With_Properties( VP_PROPERTY2 ) );
	ASSERT_TRUE( manager_tester.Has_Process_With_Properties( VP_PROPERTY3 ) );
	ASSERT_TRUE( manager_tester.Has_Process_With_Properties( VP_PROPERTY4 ) );
	ASSERT_TRUE( manager_tester.Has_Process_With_Properties( VP_PROPERTY5 ) );

	EProcessID::Enum vp_1 = manager_tester.Get_Virtual_Process_By_Property_Match( VP_PROPERTY1 )->Get_ID();
	EProcessID::Enum vp_2 = manager_tester.Get_Virtual_Process_By_Property_Match( VP_PROPERTY2 )->Get_ID();
	EProcessID::Enum vp_3 = manager_tester.Get_Virtual_Process_By_Property_Match( VP_PROPERTY3 )->Get_ID();
	EProcessID::Enum vp_4 = manager_tester.Get_Virtual_Process_By_Property_Match( VP_PROPERTY4 )->Get_ID();
	EProcessID::Enum vp_5 = manager_tester.Get_Virtual_Process_By_Property_Match( VP_PROPERTY5 )->Get_ID();
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

		typedef CTaskProcessBase BASECLASS;

		CSuicidalProcess( const SProcessProperties &properties ) :
			BASECLASS( properties ),
			HasBeenServiced( false )
		{}

		virtual ETimeType Get_Time_Type( void ) const { return TT_GAME_TIME; }
		virtual bool Is_Root_Thread( void ) const { return true; }

		virtual void Run( const CProcessExecutionContext &context )
		{
			if ( !HasBeenServiced )
			{
				Send_Manager_Message( shared_ptr< const IProcessMessage >( new CShutdownProcessMessage( Get_ID() ) ) );
				HasBeenServiced = true;
			}

			BASECLASS::Run( context );
		}

	private:

		bool HasBeenServiced;
};

static const SProcessProperties SUICIDAL_PROCESS_PROPERTIES( EManagerTestProcessSubject::DATABASE, 1, 1, 1 );

TEST_F( ConcurrencyManagerTests, Run_Once_And_Shutdown_Self )
{
	CConcurrencyManagerTester manager_tester;

	manager_tester.Setup_For_Run( shared_ptr< IManagedProcess >( new CSuicidalProcess( SUICIDAL_PROCESS_PROPERTIES ) ) );

	manager_tester.Wait_For_Shutdown();
}

TEST_F( ConcurrencyManagerTests, Stress )
{
	for ( uint32 i = 0; i < 50; ++i )
	{
		CConcurrencyManagerTester manager_tester;

		manager_tester.Setup_For_Run( shared_ptr< IManagedProcess >( new CSuicidalProcess( SUICIDAL_PROCESS_PROPERTIES ) ) );

		manager_tester.Wait_For_Shutdown();
	}
}
