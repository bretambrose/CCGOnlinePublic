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
#include "Concurrency/VirtualProcessStatics.h"
#include "Concurrency/VirtualProcessBase.h"
#include "Concurrency/VirtualProcessSubject.h"
#include "Concurrency/VirtualProcessConstants.h"
#include "Concurrency/VirtualProcessMessageFrame.h"
#include "Concurrency/Messaging/VirtualProcessManagementMessages.h"
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

class CVirtualProcessBaseExaminer
{
	public:

		CVirtualProcessBaseExaminer( const shared_ptr< CVirtualProcessBase > &virtual_process ) :
			VirtualProcess( virtual_process )
		{}

		const CVirtualProcessBase::FrameTableType &Get_Pending_Outbound_Frames( void ) const { return VirtualProcess->PendingOutboundFrames; }
		const CVirtualProcessBase::MailboxTableType &Get_Mailboxes( void ) const { return VirtualProcess->Mailboxes; }
		shared_ptr< CReadOnlyMailbox > Get_My_Mailbox( void ) const { return VirtualProcess->MyMailbox; }

		shared_ptr< CWriteOnlyMailbox > Get_Log_Mailbox( void ) const { return VirtualProcess->LoggingMailbox; }
		shared_ptr< CWriteOnlyMailbox > Get_Manager_Mailbox( void ) const { return VirtualProcess->ManagerMailbox; }

		bool Has_Mailbox_With_Properties( const SProcessProperties &properties ) const
		{
			auto mailboxes = Get_Mailboxes();
			for ( auto iter = mailboxes.cbegin(); iter != mailboxes.cend(); ++iter )
			{
				if ( properties.Matches( iter->second->Get_Properties() ) )
				{
					return true;
				}
			}

			return false;
		}

		bool Has_Mailbox( EVirtualProcessID::Enum process_id ) const
		{
			return VirtualProcess->Get_Mailbox( process_id ) != nullptr;
		}

	private:

		shared_ptr< CVirtualProcessBase > VirtualProcess;
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

		void Setup_For_Run( const shared_ptr< IManagedVirtualProcess > &virtual_process )
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

		void Add_Rescheduled_Process( EVirtualProcessID::Enum process_id ) { RescheduledProcesses.insert( process_id ); }
		void Remove_Rescheduled_Process( EVirtualProcessID::Enum process_id ) { RescheduledProcesses.erase( process_id ); }

		bool Are_Rescheduled_Processes_Finished( void )
		{
			auto thread_set_copy = RescheduledProcesses;

			std::vector< shared_ptr< CVirtualProcessMessageFrame > > frames;
			shared_ptr< CReadOnlyMailbox > read_interface = Manager->Get_My_Mailbox();
			read_interface->Remove_Frames( frames );

			// go through all frames, looking for reschedule messages
			for ( uint32 i = 0; i < frames.size(); ++i )
			{
				shared_ptr< CVirtualProcessMessageFrame > frame = frames[ i ];
				EVirtualProcessID::Enum process_id = frame->Get_Process_ID();
				for ( auto iter = frame->Get_Frame_Begin(); iter != frame->Get_Frame_End(); ++iter )
				{
					const IVirtualProcessMessage *raw_message = iter->get();
					if ( Loki::TypeInfo( typeid( *raw_message ) ) == Loki::TypeInfo( typeid( CRescheduleVirtualProcessMessage ) ) )
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

		bool Has_Process( EVirtualProcessID::Enum process_id ) const { return Manager->Get_Record( process_id ) != nullptr; }
		bool Has_Process_With_Properties( const SProcessProperties &properties ) const {
			return Get_Virtual_Process_By_Property_Match( properties ) != nullptr;
		}
		
		shared_ptr< IManagedVirtualProcess > Get_Virtual_Process( EVirtualProcessID::Enum process_id ) const { return Manager->Get_Virtual_Process( process_id ); }

		shared_ptr< IManagedVirtualProcess > Get_Virtual_Process_By_Property_Match( const SProcessProperties &properties ) const { 
			std::vector< shared_ptr< IManagedVirtualProcess > > processes;
			Manager->Enumerate_Virtual_Processes( processes );
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
				shared_ptr< CVirtualProcessMessageFrame > frame( new CVirtualProcessMessageFrame( MANAGER_PROCESS_ID ) );
				frame->Add_Message( shared_ptr< const IVirtualProcessMessage >( new CShutdownManagerMessage ) );

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

		const CConcurrencyManager::FrameTableType &Get_Pending_Outbound_Frames( void ) const { return Manager->PendingOutboundFrames; }

		shared_ptr< CWriteOnlyMailbox > Get_Manager_Mailbox( void ) const { return Manager->Get_Mailbox( MANAGER_PROCESS_ID ); }

	private:

		scoped_ptr< CConcurrencyManager > Manager;

		std::set< EVirtualProcessID::Enum > RescheduledProcesses;
};

class CDoNothingVirtualProcess : public CVirtualProcessBase
{
	public:

		typedef CVirtualProcessBase BASECLASS;

		CDoNothingVirtualProcess( const SProcessProperties &properties ) :
			BASECLASS( properties ),
			ServiceCount( 0 )
		{}

		virtual ETimeType Get_Time_Type( void ) const { return TT_GAME_TIME; }
		virtual bool Is_Root_Thread( void ) const { return true; }

		virtual void Service( double elapsed_seconds, const CVirtualProcessExecutionContext &context )
		{
			BASECLASS::Service( elapsed_seconds, context );

			ServiceCount++;
		}

		uint32 Get_Service_Count( void ) const { return ServiceCount; }

	private:

		uint32 ServiceCount;
};

namespace EManagerTestVirtualProcessSubject
{
	enum Enum
	{
		AI = EVirtualProcessSubject::NEXT_FREE_VALUE,
		DATABASE,
		UI
	};
}

static const EVirtualProcessID::Enum AI_PROCESS_ID( EVirtualProcessID::FIRST_FREE_ID );
static const SProcessProperties AI_PROPS( EManagerTestVirtualProcessSubject::AI );

TEST_F( ConcurrencyManagerTests, Setup )
{
	CConcurrencyManagerTester manager_tester;

	manager_tester.Setup_For_Run( shared_ptr< IManagedVirtualProcess >( new CDoNothingVirtualProcess( AI_PROPS ) ) );

	manager_tester.Add_Rescheduled_Process( AI_PROCESS_ID );

	// Verify setup state
	// 3 thread records: manager, log, test_key
	ASSERT_TRUE( manager_tester.Has_Process( AI_PROCESS_ID ) );
	ASSERT_TRUE( manager_tester.Has_Process( MANAGER_PROCESS_ID ) );
	ASSERT_TRUE( manager_tester.Has_Process( LOGGING_PROCESS_ID ) );

	// process should have a log interface
	CVirtualProcessBaseExaminer test_examiner( static_pointer_cast< CVirtualProcessBase >( manager_tester.Get_Virtual_Process( AI_PROCESS_ID ) ) );
	auto mailboxes = test_examiner.Get_Mailboxes();
	ASSERT_TRUE( mailboxes.size() == 0 );
	ASSERT_TRUE( test_examiner.Get_Log_Mailbox().get() != nullptr );

	manager_tester.Shutdown();
}



static const SProcessProperties VP_PROPERTY1( EManagerTestVirtualProcessSubject::AI, 1, 1, 1 );
static const SProcessProperties VP_PROPERTY2( EManagerTestVirtualProcessSubject::AI, 1, 2, 1 );
static const SProcessProperties VP_PROPERTY3( EManagerTestVirtualProcessSubject::AI, 1, 3, 4 );
static const SProcessProperties VP_PROPERTY4( EManagerTestVirtualProcessSubject::AI, 2, 1, 2 );
static const SProcessProperties VP_PROPERTY5( EManagerTestVirtualProcessSubject::AI, 1, 1, 3 );

class CMailboxTestThreadTask : public CDoNothingVirtualProcess
{
	public:

		typedef CDoNothingVirtualProcess BASECLASS;

		CMailboxTestThreadTask( const SProcessProperties &properties ) :
			BASECLASS( properties ),
			HasBeenServiced( false )
		{}

		virtual ETimeType Get_Time_Type( void ) const { return TT_GAME_TIME; }
		virtual bool Is_Root_Thread( void ) const { return true; }

		virtual void Service( double elapsed_seconds, const CVirtualProcessExecutionContext &context )
		{
			if ( !HasBeenServiced )
			{
				HasBeenServiced = true;

				const SProcessProperties &properties = Get_Properties();
				if ( properties == VP_PROPERTY1 )
				{
					Send_Manager_Message( shared_ptr< const IVirtualProcessMessage >( new CGetMailboxByPropertiesRequest( VP_PROPERTY2 ) ) );
				}
				else if ( properties == VP_PROPERTY2 )
				{
					SProcessProperties multimatch( EManagerTestVirtualProcessSubject::AI, 1, 0, 0 );
					Send_Manager_Message( shared_ptr< const IVirtualProcessMessage >( new CGetMailboxByPropertiesRequest( multimatch ) ) );
				}
				else if ( properties == VP_PROPERTY3 )
				{
					SProcessProperties multimatch( EManagerTestVirtualProcessSubject::AI, 2, 0, 0 );
					Send_Manager_Message( shared_ptr< const IVirtualProcessMessage >( new CGetMailboxByPropertiesRequest( multimatch ) ) );
				}
				else if ( properties == VP_PROPERTY4 )
				{
					Send_Manager_Message( shared_ptr< const IVirtualProcessMessage >( new CGetMailboxByIDRequest( EVirtualProcessID::FIRST_FREE_ID ) ) );
				}
				else if ( properties == VP_PROPERTY5 )
				{
					Send_Manager_Message( shared_ptr< const IVirtualProcessMessage >( new CGetMailboxByIDRequest( static_cast< EVirtualProcessID::Enum >( EVirtualProcessID::FIRST_FREE_ID + 50 ) ) ) );
				}
			}

			BASECLASS::Service( elapsed_seconds, context );
		}

	private:

		bool HasBeenServiced;
};

class CSpawnMailboxPushGetVirtualProcess : public CVirtualProcessBase
{
	public:

		typedef CVirtualProcessBase BASECLASS;

		CSpawnMailboxPushGetVirtualProcess( const SProcessProperties &properties ) :
			BASECLASS( properties ),
			HasBeenServiced( false )
		{}

		virtual ETimeType Get_Time_Type( void ) const { return TT_GAME_TIME; }
		virtual bool Is_Root_Thread( void ) const { return true; }

		virtual void Service( double elapsed_seconds, const CVirtualProcessExecutionContext &context )
		{
			if ( !HasBeenServiced )
			{
				Send_Manager_Message( shared_ptr< const IVirtualProcessMessage >( new CAddNewVirtualProcessMessage( shared_ptr< IVirtualProcess >( new CMailboxTestThreadTask( VP_PROPERTY1 ) ), true, false ) ) );
				Send_Manager_Message( shared_ptr< const IVirtualProcessMessage >( new CAddNewVirtualProcessMessage( shared_ptr< IVirtualProcess >( new CMailboxTestThreadTask( VP_PROPERTY2 ) ), false, true ) ) );
				Send_Manager_Message( shared_ptr< const IVirtualProcessMessage >( new CAddNewVirtualProcessMessage( shared_ptr< IVirtualProcess >( new CMailboxTestThreadTask( VP_PROPERTY3 ) ), false, true ) ) );
				Send_Manager_Message( shared_ptr< const IVirtualProcessMessage >( new CAddNewVirtualProcessMessage( shared_ptr< IVirtualProcess >( new CMailboxTestThreadTask( VP_PROPERTY4 ) ), false, false ) ) );
				Send_Manager_Message( shared_ptr< const IVirtualProcessMessage >( new CAddNewVirtualProcessMessage( shared_ptr< IVirtualProcess >( new CMailboxTestThreadTask( VP_PROPERTY5 ) ), false, false ) ) );

				HasBeenServiced = true;
			}

			BASECLASS::Service( elapsed_seconds, context );
		}

	private:

		bool HasBeenServiced;
};

static const SProcessProperties SPAWN_PROCESS_PROPERTIES( EManagerTestVirtualProcessSubject::DATABASE, 1, 1, 1 );

void Verify_Interfaces_Present( const CConcurrencyManagerTester &manager_tester )
{
	CVirtualProcessBaseExaminer spawn_process( static_pointer_cast< CVirtualProcessBase >( manager_tester.Get_Virtual_Process_By_Property_Match( SPAWN_PROCESS_PROPERTIES ) ) );
	auto mailboxes = spawn_process.Get_Mailboxes();
	ASSERT_TRUE( mailboxes.size() == 1 );
	ASSERT_TRUE( spawn_process.Has_Mailbox_With_Properties( VP_PROPERTY1 ) );
	ASSERT_TRUE( spawn_process.Get_Log_Mailbox().get() != nullptr );
	ASSERT_TRUE( spawn_process.Get_Manager_Mailbox().get() != nullptr );

	CVirtualProcessBaseExaminer test_process1( static_pointer_cast< CVirtualProcessBase >( manager_tester.Get_Virtual_Process_By_Property_Match( VP_PROPERTY1 ) ) );
	mailboxes = test_process1.Get_Mailboxes();
	ASSERT_TRUE( mailboxes.size() == 1 );
	ASSERT_TRUE( test_process1.Has_Mailbox_With_Properties( VP_PROPERTY2 ) );
	ASSERT_TRUE( test_process1.Get_Log_Mailbox().get() != nullptr );
	ASSERT_TRUE( test_process1.Get_Manager_Mailbox().get() != nullptr );

	CVirtualProcessBaseExaminer test_process2( static_pointer_cast< CVirtualProcessBase >( manager_tester.Get_Virtual_Process_By_Property_Match( VP_PROPERTY2 ) ) );
	mailboxes = test_process2.Get_Mailboxes();
	ASSERT_TRUE( mailboxes.size() == 4 );
	ASSERT_TRUE( test_process2.Has_Mailbox_With_Properties( SPAWN_PROCESS_PROPERTIES ) );
	ASSERT_TRUE( test_process2.Has_Mailbox_With_Properties( VP_PROPERTY1 ) );
	ASSERT_TRUE( test_process2.Has_Mailbox_With_Properties( VP_PROPERTY3 ) );
	ASSERT_TRUE( test_process2.Has_Mailbox_With_Properties( VP_PROPERTY5 ) );
	ASSERT_TRUE( test_process2.Get_Log_Mailbox().get() != nullptr );
	ASSERT_TRUE( test_process2.Get_Manager_Mailbox().get() != nullptr );

	CVirtualProcessBaseExaminer test_process3( static_pointer_cast< CVirtualProcessBase >( manager_tester.Get_Virtual_Process_By_Property_Match( VP_PROPERTY3 ) ) );
	mailboxes = test_process3.Get_Mailboxes();
	ASSERT_TRUE( mailboxes.size() == 2 );
	ASSERT_TRUE( test_process3.Has_Mailbox_With_Properties( SPAWN_PROCESS_PROPERTIES ) );
	ASSERT_TRUE( test_process3.Has_Mailbox_With_Properties( VP_PROPERTY4 ) );
	ASSERT_TRUE( test_process3.Get_Log_Mailbox().get() != nullptr );
	ASSERT_TRUE( test_process3.Get_Manager_Mailbox().get() != nullptr );

	CVirtualProcessBaseExaminer test_process4( static_pointer_cast< CVirtualProcessBase >( manager_tester.Get_Virtual_Process_By_Property_Match( VP_PROPERTY4 ) ) );
	mailboxes = test_process4.Get_Mailboxes();
	ASSERT_TRUE( mailboxes.size() == 1 );
	ASSERT_TRUE( test_process4.Has_Mailbox( EVirtualProcessID::FIRST_FREE_ID ) );
	ASSERT_TRUE( test_process4.Get_Log_Mailbox().get() != nullptr );
	ASSERT_TRUE( test_process4.Get_Manager_Mailbox().get() != nullptr );

	CVirtualProcessBaseExaminer test_process5( static_pointer_cast< CVirtualProcessBase >( manager_tester.Get_Virtual_Process_By_Property_Match( VP_PROPERTY5 ) ) );
	mailboxes = test_process5.Get_Mailboxes();
	ASSERT_TRUE( mailboxes.size() == 0 );
	ASSERT_TRUE( test_process5.Get_Log_Mailbox().get() != nullptr );
	ASSERT_TRUE( test_process5.Get_Manager_Mailbox().get() != nullptr );
}

TEST_F( ConcurrencyManagerTests, Interface_Get1 )
{
	CConcurrencyManagerTester manager_tester;

	manager_tester.Setup_For_Run( shared_ptr< IManagedVirtualProcess >( new CSpawnMailboxPushGetVirtualProcess( SPAWN_PROCESS_PROPERTIES ) ) );

	EVirtualProcessID::Enum spawn_id = manager_tester.Get_Virtual_Process_By_Property_Match( SPAWN_PROCESS_PROPERTIES )->Get_ID();
	manager_tester.Add_Rescheduled_Process( spawn_id );
	manager_tester.Run_One_Iteration();
	manager_tester.Run_One_Iteration();

	ASSERT_TRUE( manager_tester.Has_Process( spawn_id ) );
	ASSERT_TRUE( manager_tester.Has_Process_With_Properties( VP_PROPERTY1 ) );
	ASSERT_TRUE( manager_tester.Has_Process_With_Properties( VP_PROPERTY2 ) );
	ASSERT_TRUE( manager_tester.Has_Process_With_Properties( VP_PROPERTY3 ) );
	ASSERT_TRUE( manager_tester.Has_Process_With_Properties( VP_PROPERTY4 ) );
	ASSERT_TRUE( manager_tester.Has_Process_With_Properties( VP_PROPERTY5 ) );

	EVirtualProcessID::Enum vp_1 = manager_tester.Get_Virtual_Process_By_Property_Match( VP_PROPERTY1 )->Get_ID();
	EVirtualProcessID::Enum vp_2 = manager_tester.Get_Virtual_Process_By_Property_Match( VP_PROPERTY2 )->Get_ID();
	EVirtualProcessID::Enum vp_3 = manager_tester.Get_Virtual_Process_By_Property_Match( VP_PROPERTY3 )->Get_ID();
	EVirtualProcessID::Enum vp_4 = manager_tester.Get_Virtual_Process_By_Property_Match( VP_PROPERTY4 )->Get_ID();
	EVirtualProcessID::Enum vp_5 = manager_tester.Get_Virtual_Process_By_Property_Match( VP_PROPERTY5 )->Get_ID();
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

class CSuicidalVirtualProcess : public CVirtualProcessBase
{
	public:

		typedef CVirtualProcessBase BASECLASS;

		CSuicidalVirtualProcess( const SProcessProperties &properties ) :
			BASECLASS( properties ),
			HasBeenServiced( false )
		{}

		virtual ETimeType Get_Time_Type( void ) const { return TT_GAME_TIME; }
		virtual bool Is_Root_Thread( void ) const { return true; }

		virtual void Service( double elapsed_seconds, const CVirtualProcessExecutionContext &context )
		{
			if ( !HasBeenServiced )
			{
				Send_Manager_Message( shared_ptr< const IVirtualProcessMessage >( new CShutdownVirtualProcessMessage( Get_ID() ) ) );
				HasBeenServiced = true;
			}

			BASECLASS::Service( elapsed_seconds, context );
		}

	private:

		bool HasBeenServiced;
};

static const SProcessProperties SUICIDAL_PROCESS_PROPERTIES( EManagerTestVirtualProcessSubject::DATABASE, 1, 1, 1 );

TEST_F( ConcurrencyManagerTests, Run_Once_And_Shutdown_Self )
{
	CConcurrencyManagerTester manager_tester;

	manager_tester.Setup_For_Run( shared_ptr< IManagedVirtualProcess >( new CSuicidalVirtualProcess( SUICIDAL_PROCESS_PROPERTIES ) ) );

	manager_tester.Wait_For_Shutdown();
}

TEST_F( ConcurrencyManagerTests, Stress )
{
	for ( uint32 i = 0; i < 50; ++i )
	{
		CConcurrencyManagerTester manager_tester;

		manager_tester.Setup_For_Run( shared_ptr< IManagedVirtualProcess >( new CSuicidalVirtualProcess( SUICIDAL_PROCESS_PROPERTIES ) ) );

		manager_tester.Wait_For_Shutdown();
	}
}
