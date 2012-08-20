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
#include "Concurrency/ThreadSubject.h"
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

		shared_ptr< CWriteOnlyMailbox > Get_Log_Mailbox( void ) const { return VirtualProcess->LogMailbox; }
		shared_ptr< CWriteOnlyMailbox > Get_Manager_Mailbox( void ) const { return VirtualProcess->ManagerMailbox; }

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
			CLogInterface::Shutdown_Dynamic();
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

		void Add_Rescheduled_Process( const SThreadKey &key ) { RescheduledProcesses.insert( key ); }
		void Remove_Rescheduled_Process( const SThreadKey &key ) { RescheduledProcesses.erase( key ); }

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
				for ( auto iter = frame->Get_Frame_Begin(); iter != frame->Get_Frame_End(); ++iter )
				{
					const IVirtualProcessMessage *raw_message = iter->get();
					if ( Loki::TypeInfo( typeid( *raw_message ) ) == Loki::TypeInfo( typeid( CRescheduleVirtualProcessMessage ) ) )
					{
						const CRescheduleVirtualProcessMessage *reschedule_message = static_cast< const CRescheduleVirtualProcessMessage * >( raw_message );
						if ( thread_set_copy.find( reschedule_message->Get_Key() ) != thread_set_copy.end() )
						{
							thread_set_copy.erase( reschedule_message->Get_Key() );
						}
					}
				}
			}

			// restore all the frames in the manager mailbox
			shared_ptr< CWriteOnlyMailbox > write_interface = Manager->Get_Mailbox( MANAGER_THREAD_KEY );
			for ( uint32 i = 0; i < frames.size(); ++i )
			{
				write_interface->Add_Frame( frames[ i ] );
			}

			return thread_set_copy.size() == 0;
		}

		bool Has_Process( const SThreadKey &key ) const { return Manager->Get_Record( key ) != nullptr; }
		
		shared_ptr< IManagedVirtualProcess > Get_Virtual_Process( const SThreadKey &key ) const { return Manager->Get_Virtual_Process( key ); }

		void Shutdown( void )
		{
			if ( Manager->ProcessRecords.size() > 0 )
			{
				shared_ptr< CVirtualProcessMessageFrame > frame( new CVirtualProcessMessageFrame( MANAGER_THREAD_KEY ) );
				frame->Add_Message( shared_ptr< const IVirtualProcessMessage >( new CShutdownManagerMessage ) );

				shared_ptr< CWriteOnlyMailbox > write_interface = Manager->Get_Mailbox( MANAGER_THREAD_KEY );
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

		shared_ptr< CWriteOnlyMailbox > Get_Manager_Mailbox( void ) const { return Manager->Get_Mailbox( MANAGER_THREAD_KEY ); }

		size_t Get_Pending_Interface_Request_Count( void ) const { return Manager->UnfulfilledPushRequests.size() + Manager->UnfulfilledGetRequests.size(); }

	private:

		scoped_ptr< CConcurrencyManager > Manager;

		std::set< SThreadKey, SThreadKeyContainerHelper > RescheduledProcesses;
};

class CDoNothingVirtualProcess : public CVirtualProcessBase
{
	public:

		typedef CVirtualProcessBase BASECLASS;

		CDoNothingVirtualProcess( const SThreadKey &key ) :
			BASECLASS( key ),
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

TEST_F( ConcurrencyManagerTests, Setup )
{
	CConcurrencyManagerTester manager_tester;

	SThreadKey test_key( TS_AI, 1, 1 );
	manager_tester.Setup_For_Run( shared_ptr< IManagedVirtualProcess >( new CDoNothingVirtualProcess( test_key ) ) );

	manager_tester.Add_Rescheduled_Process( test_key );

	// Verify setup state
	// 3 thread records: manager, log, test_key
	ASSERT_TRUE( manager_tester.Has_Process( test_key ) );
	ASSERT_TRUE( manager_tester.Has_Process( MANAGER_THREAD_KEY ) );
	ASSERT_TRUE( manager_tester.Has_Process( LOG_THREAD_KEY ) );

	// 1 push req: log
	auto outbound_frames = manager_tester.Get_Pending_Outbound_Frames();
	ASSERT_TRUE( outbound_frames.size() == 1 );
	ASSERT_TRUE( outbound_frames.find( test_key ) != outbound_frames.end() );

	shared_ptr< CVirtualProcessMessageFrame > frame = outbound_frames[ test_key ];
	for ( auto frame_iter = frame->Get_Frame_Begin(); frame_iter != frame->Get_Frame_End(); ++frame_iter )
	{
		const IVirtualProcessMessage *raw_message = frame_iter->get();
		ASSERT_TRUE( Loki::TypeInfo( typeid( *raw_message ) ) == Loki::TypeInfo( typeid( CAddMailboxMessage ) ) );
	}

	// Run a single frame
	manager_tester.Run_One_Iteration();
	
	// Verify post-run state
	// thread should have been run once and have a log interface
	CVirtualProcessBaseExaminer test_examiner( static_pointer_cast< CVirtualProcessBase >( manager_tester.Get_Virtual_Process( test_key ) ) );
	auto mailboxes = test_examiner.Get_Mailboxes();
	ASSERT_TRUE( mailboxes.size() == 0 );
	ASSERT_TRUE( test_examiner.Get_Log_Mailbox().get() != nullptr );

	shared_ptr< CDoNothingVirtualProcess > do_nothing_process = static_pointer_cast< CDoNothingVirtualProcess >( manager_tester.Get_Virtual_Process( test_key ) );
	ASSERT_TRUE( do_nothing_process->Get_Service_Count() == 1 );

	manager_tester.Shutdown();
}



static const SThreadKey PUSH_GET_KEY1( TS_AI, 1, 1 );
static const SThreadKey PUSH_GET_KEY2( TS_AI, 1, 2 );
static const SThreadKey PUSH_GET_KEY3( TS_AI, 1, 3 );
static const SThreadKey PUSH_GET_KEY4( TS_AI, 2, 1 );
static const SThreadKey PUSH_GET_KEY5( TS_UI, 1, 1 );

class CMailboxTestThreadTask : public CDoNothingVirtualProcess
{
	public:

		typedef CDoNothingVirtualProcess BASECLASS;

		CMailboxTestThreadTask( const SThreadKey &key ) :
			BASECLASS( key ),
			HasBeenServiced( false )
		{}

		virtual ETimeType Get_Time_Type( void ) const { return TT_GAME_TIME; }
		virtual bool Is_Root_Thread( void ) const { return true; }

		virtual void Service( double elapsed_seconds, const CVirtualProcessExecutionContext &context )
		{
			if ( !HasBeenServiced )
			{
				HasBeenServiced = true;

				const SThreadKey &key = Get_Key();
				if ( key == PUSH_GET_KEY1 )
				{
					Send_Virtual_Process_Message( MANAGER_THREAD_KEY, shared_ptr< const IVirtualProcessMessage >( new CGetMailboxRequest( key, PUSH_GET_KEY4 ) ) );
				}
				else if ( key == PUSH_GET_KEY2 )
				{
					Send_Virtual_Process_Message( MANAGER_THREAD_KEY, shared_ptr< const IVirtualProcessMessage >( new CGetMailboxRequest( key, PUSH_GET_KEY4 ) ) );
				}
				else if ( key == PUSH_GET_KEY3 )
				{
					Send_Virtual_Process_Message( MANAGER_THREAD_KEY, shared_ptr< const IVirtualProcessMessage >( new CGetMailboxRequest( key, PUSH_GET_KEY4 ) ) );
				}
				else if ( key == PUSH_GET_KEY4 )
				{
					Send_Virtual_Process_Message( MANAGER_THREAD_KEY, shared_ptr< const IVirtualProcessMessage >( new CPushMailboxRequest( key, PUSH_GET_KEY5 ) ) );
				}
				else if ( key == PUSH_GET_KEY5 )
				{
					Send_Virtual_Process_Message( MANAGER_THREAD_KEY, shared_ptr< const IVirtualProcessMessage >( new CPushMailboxRequest( key, SThreadKey( TS_AI, MAJOR_KEY_ALL, MINOR_KEY_ALL ) ) ) );
					Send_Virtual_Process_Message( MANAGER_THREAD_KEY, shared_ptr< const IVirtualProcessMessage >( new CGetMailboxRequest( key, SThreadKey( TS_AI, 1, MINOR_KEY_ALL ) ) ) );
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

		CSpawnMailboxPushGetVirtualProcess( const SThreadKey &key ) :
			BASECLASS( key ),
			HasBeenServiced( false )
		{}

		virtual ETimeType Get_Time_Type( void ) const { return TT_GAME_TIME; }
		virtual bool Is_Root_Thread( void ) const { return true; }

		virtual void Service( double elapsed_seconds, const CVirtualProcessExecutionContext &context )
		{
			if ( !HasBeenServiced )
			{
				CVirtualProcessStatics::Get_Current_Virtual_Process()->Send_Virtual_Process_Message( MANAGER_THREAD_KEY, 
																																 shared_ptr< const IVirtualProcessMessage >( new CAddNewVirtualProcessMessage( 
																																	shared_ptr< IVirtualProcess >( new CMailboxTestThreadTask( PUSH_GET_KEY1 ) ), false, false ) ) );

				CVirtualProcessStatics::Get_Current_Virtual_Process()->Send_Virtual_Process_Message( MANAGER_THREAD_KEY, 
																																 shared_ptr< const IVirtualProcessMessage >( new CAddNewVirtualProcessMessage( 
																																	shared_ptr< IVirtualProcess >( new CMailboxTestThreadTask( PUSH_GET_KEY2 ) ), false, false ) ) );

				CVirtualProcessStatics::Get_Current_Virtual_Process()->Send_Virtual_Process_Message( MANAGER_THREAD_KEY, 
																																 shared_ptr< const IVirtualProcessMessage >( new CAddNewVirtualProcessMessage( 
																																	shared_ptr< IVirtualProcess >( new CMailboxTestThreadTask( PUSH_GET_KEY3 ) ), false, false ) ) );

				CVirtualProcessStatics::Get_Current_Virtual_Process()->Send_Virtual_Process_Message( MANAGER_THREAD_KEY, 
																																 shared_ptr< const IVirtualProcessMessage >( new CAddNewVirtualProcessMessage( 
																																	shared_ptr< IVirtualProcess >( new CMailboxTestThreadTask( PUSH_GET_KEY4 ) ), false, false ) ) );

				CVirtualProcessStatics::Get_Current_Virtual_Process()->Send_Virtual_Process_Message( MANAGER_THREAD_KEY, 
																																 shared_ptr< const IVirtualProcessMessage >( new CAddNewVirtualProcessMessage( 
																																	shared_ptr< IVirtualProcess >( new CMailboxTestThreadTask( PUSH_GET_KEY5 ) ), false, false ) ) );

				HasBeenServiced = true;
			}

			BASECLASS::Service( elapsed_seconds, context );
		}

	private:

		bool HasBeenServiced;
};

void Verify_Interfaces_Present( const CConcurrencyManagerTester &manager_tester )
{
	CVirtualProcessBaseExaminer test_process1( static_pointer_cast< CVirtualProcessBase >( manager_tester.Get_Virtual_Process( PUSH_GET_KEY1 ) ) );
	auto mailboxes = test_process1.Get_Mailboxes();
	ASSERT_TRUE( mailboxes.size() == 2 );
	ASSERT_TRUE( mailboxes.find( PUSH_GET_KEY4 ) != mailboxes.end() );
	ASSERT_TRUE( mailboxes.find( PUSH_GET_KEY5 ) != mailboxes.end() );
	ASSERT_TRUE( test_process1.Get_Log_Mailbox().get() != nullptr );
	ASSERT_TRUE( test_process1.Get_Manager_Mailbox().get() != nullptr );

	CVirtualProcessBaseExaminer test_process2( static_pointer_cast< CVirtualProcessBase >( manager_tester.Get_Virtual_Process( PUSH_GET_KEY2 ) ) );
	mailboxes = test_process2.Get_Mailboxes();
	ASSERT_TRUE( mailboxes.size() == 2 );
	ASSERT_TRUE( mailboxes.find( PUSH_GET_KEY4 ) != mailboxes.end() );
	ASSERT_TRUE( mailboxes.find( PUSH_GET_KEY5 ) != mailboxes.end() );
	ASSERT_TRUE( test_process2.Get_Log_Mailbox().get() != nullptr );
	ASSERT_TRUE( test_process2.Get_Manager_Mailbox().get() != nullptr );

	CVirtualProcessBaseExaminer test_process3( static_pointer_cast< CVirtualProcessBase >( manager_tester.Get_Virtual_Process( PUSH_GET_KEY3 ) ) );
	mailboxes = test_process3.Get_Mailboxes();
	ASSERT_TRUE( mailboxes.size() == 2 );
	ASSERT_TRUE( mailboxes.find( PUSH_GET_KEY4 ) != mailboxes.end() );
	ASSERT_TRUE( mailboxes.find( PUSH_GET_KEY5 ) != mailboxes.end() );
	ASSERT_TRUE( test_process3.Get_Log_Mailbox().get() != nullptr );
	ASSERT_TRUE( test_process3.Get_Manager_Mailbox().get() != nullptr );

	CVirtualProcessBaseExaminer test_process4( static_pointer_cast< CVirtualProcessBase >( manager_tester.Get_Virtual_Process( PUSH_GET_KEY4 ) ) );
	mailboxes = test_process4.Get_Mailboxes();
	ASSERT_TRUE( mailboxes.size() == 1 );
	ASSERT_TRUE( mailboxes.find( PUSH_GET_KEY5 ) != mailboxes.end() );
	ASSERT_TRUE( test_process4.Get_Log_Mailbox().get() != nullptr );
	ASSERT_TRUE( test_process4.Get_Manager_Mailbox().get() != nullptr );

	CVirtualProcessBaseExaminer test_process5( static_pointer_cast< CVirtualProcessBase >( manager_tester.Get_Virtual_Process( PUSH_GET_KEY5 ) ) );
	mailboxes = test_process5.Get_Mailboxes();
	ASSERT_TRUE( mailboxes.size() == 4 );
	ASSERT_TRUE( mailboxes.find( PUSH_GET_KEY1 ) != mailboxes.end() );
	ASSERT_TRUE( mailboxes.find( PUSH_GET_KEY2 ) != mailboxes.end() );
	ASSERT_TRUE( mailboxes.find( PUSH_GET_KEY3 ) != mailboxes.end() );
	ASSERT_TRUE( mailboxes.find( PUSH_GET_KEY4 ) != mailboxes.end() );
	ASSERT_TRUE( test_process5.Get_Log_Mailbox().get() != nullptr );
	ASSERT_TRUE( test_process5.Get_Manager_Mailbox().get() != nullptr );
}

TEST_F( ConcurrencyManagerTests, Interface_Push_Get1 )
{
	CConcurrencyManagerTester manager_tester;

	SThreadKey test_key( TS_LOGIC, 1, 1 );
	manager_tester.Setup_For_Run( shared_ptr< IManagedVirtualProcess >( new CSpawnMailboxPushGetVirtualProcess( test_key ) ) );

	manager_tester.Add_Rescheduled_Process( test_key );
	manager_tester.Run_One_Iteration();
	manager_tester.Run_One_Iteration();

	ASSERT_TRUE( manager_tester.Has_Process( test_key ) );
	ASSERT_TRUE( manager_tester.Has_Process( PUSH_GET_KEY1 ) );
	ASSERT_TRUE( manager_tester.Has_Process( PUSH_GET_KEY2 ) );
	ASSERT_TRUE( manager_tester.Has_Process( PUSH_GET_KEY3 ) );
	ASSERT_TRUE( manager_tester.Has_Process( PUSH_GET_KEY4 ) );
	ASSERT_TRUE( manager_tester.Has_Process( PUSH_GET_KEY5 ) );

	manager_tester.Add_Rescheduled_Process( PUSH_GET_KEY1 );
	manager_tester.Add_Rescheduled_Process( PUSH_GET_KEY2 );
	manager_tester.Add_Rescheduled_Process( PUSH_GET_KEY3 );
	manager_tester.Add_Rescheduled_Process( PUSH_GET_KEY4 );
	manager_tester.Add_Rescheduled_Process( PUSH_GET_KEY5 );
	manager_tester.Run_One_Iteration();

	manager_tester.Run_One_Iteration();

	Verify_Interfaces_Present( manager_tester );

	ASSERT_TRUE( manager_tester.Get_Pending_Interface_Request_Count() == 0 );

	manager_tester.Shutdown();
}

class CSpawnMailboxPushGetVirtualProcess2 : public CVirtualProcessBase
{
	public:

		typedef CVirtualProcessBase BASECLASS;

		CSpawnMailboxPushGetVirtualProcess2( const SThreadKey &key ) :
			BASECLASS( key ),
			HasBeenServiced( false )
		{}

		virtual ETimeType Get_Time_Type( void ) const { return TT_GAME_TIME; }
		virtual bool Is_Root_Thread( void ) const { return true; }

		virtual void Service( double elapsed_seconds, const CVirtualProcessExecutionContext &context )
		{
			if ( !HasBeenServiced )
			{
				CVirtualProcessStatics::Get_Current_Virtual_Process()->Send_Virtual_Process_Message( MANAGER_THREAD_KEY, 
																																 shared_ptr< const IVirtualProcessMessage >( new CAddNewVirtualProcessMessage( 
																																		shared_ptr< IVirtualProcess >( new CDoNothingVirtualProcess( PUSH_GET_KEY1 ) ), false, false ) ) );

				CVirtualProcessStatics::Get_Current_Virtual_Process()->Send_Virtual_Process_Message( MANAGER_THREAD_KEY, 
																																 shared_ptr< const IVirtualProcessMessage >( new CAddNewVirtualProcessMessage( 
																																		shared_ptr< IVirtualProcess >( new CDoNothingVirtualProcess( PUSH_GET_KEY2 ) ), false, false ) ) );

				CVirtualProcessStatics::Get_Current_Virtual_Process()->Send_Virtual_Process_Message( MANAGER_THREAD_KEY, 
																																 shared_ptr< const IVirtualProcessMessage >( new CAddNewVirtualProcessMessage( 
																																		shared_ptr< IVirtualProcess >( new CDoNothingVirtualProcess( PUSH_GET_KEY3 ) ), false, false ) ) );

				CVirtualProcessStatics::Get_Current_Virtual_Process()->Send_Virtual_Process_Message( MANAGER_THREAD_KEY, 
																																 shared_ptr< const IVirtualProcessMessage >( new CAddNewVirtualProcessMessage( 
																																		shared_ptr< IVirtualProcess >( new CDoNothingVirtualProcess( PUSH_GET_KEY4 ) ), false, false ) ) );

				CVirtualProcessStatics::Get_Current_Virtual_Process()->Send_Virtual_Process_Message( MANAGER_THREAD_KEY, 
																																 shared_ptr< const IVirtualProcessMessage >( new CAddNewVirtualProcessMessage( 
																																		shared_ptr< IVirtualProcess >( new CDoNothingVirtualProcess( PUSH_GET_KEY5 ) ), false, false ) ) );

				HasBeenServiced = true;
			}

			BASECLASS::Service( elapsed_seconds, context );
		}

	private:

		bool HasBeenServiced;
};

TEST_F( ConcurrencyManagerTests, Interface_Push_Get2 )
{
	CConcurrencyManagerTester manager_tester;

	SThreadKey test_key( TS_LOGIC, 1, 1 );
	manager_tester.Setup_For_Run( shared_ptr< IManagedVirtualProcess >( new CSpawnMailboxPushGetVirtualProcess2( test_key ) ) );

	shared_ptr< CVirtualProcessMessageFrame > frame( new CVirtualProcessMessageFrame( MANAGER_THREAD_KEY ) );

	frame->Add_Message( shared_ptr< const IVirtualProcessMessage >( new CGetMailboxRequest( PUSH_GET_KEY1, PUSH_GET_KEY4 ) ) );
	frame->Add_Message( shared_ptr< const IVirtualProcessMessage >( new CGetMailboxRequest( PUSH_GET_KEY2, PUSH_GET_KEY4 ) ) );
	frame->Add_Message( shared_ptr< const IVirtualProcessMessage >( new CGetMailboxRequest( PUSH_GET_KEY3, PUSH_GET_KEY4 ) ) );
	frame->Add_Message( shared_ptr< const IVirtualProcessMessage >( new CPushMailboxRequest( PUSH_GET_KEY4, PUSH_GET_KEY5 ) ) );
	frame->Add_Message( shared_ptr< const IVirtualProcessMessage >( new CPushMailboxRequest( PUSH_GET_KEY5, SThreadKey( TS_AI, MAJOR_KEY_ALL, MINOR_KEY_ALL ) ) ) );
	frame->Add_Message( shared_ptr< const IVirtualProcessMessage >( new CGetMailboxRequest( PUSH_GET_KEY5, SThreadKey( TS_AI, 1, MINOR_KEY_ALL ) ) ) );

	auto write_interface = manager_tester.Get_Manager_Mailbox();
	write_interface->Add_Frame( frame );

	manager_tester.Add_Rescheduled_Process( test_key );
	manager_tester.Run_One_Iteration();

	ASSERT_TRUE( manager_tester.Get_Pending_Interface_Request_Count() == 4 );

	manager_tester.Run_One_Iteration();

	ASSERT_TRUE( manager_tester.Has_Process( test_key ) );
	ASSERT_TRUE( manager_tester.Has_Process( PUSH_GET_KEY1 ) );
	ASSERT_TRUE( manager_tester.Has_Process( PUSH_GET_KEY2 ) );
	ASSERT_TRUE( manager_tester.Has_Process( PUSH_GET_KEY3 ) );
	ASSERT_TRUE( manager_tester.Has_Process( PUSH_GET_KEY4 ) );
	ASSERT_TRUE( manager_tester.Has_Process( PUSH_GET_KEY5 ) );

	manager_tester.Add_Rescheduled_Process( PUSH_GET_KEY1 );
	manager_tester.Add_Rescheduled_Process( PUSH_GET_KEY2 );
	manager_tester.Add_Rescheduled_Process( PUSH_GET_KEY3 );
	manager_tester.Add_Rescheduled_Process( PUSH_GET_KEY4 );
	manager_tester.Add_Rescheduled_Process( PUSH_GET_KEY5 );
	manager_tester.Run_One_Iteration();

	manager_tester.Run_One_Iteration();

	Verify_Interfaces_Present( manager_tester );

	ASSERT_TRUE( manager_tester.Get_Pending_Interface_Request_Count() == 0 );

	manager_tester.Shutdown();
}

class CSuicidalVirtualProcess : public CVirtualProcessBase
{
	public:

		typedef CVirtualProcessBase BASECLASS;

		CSuicidalVirtualProcess( const SThreadKey &key ) :
			BASECLASS( key ),
			HasBeenServiced( false )
		{}

		virtual ETimeType Get_Time_Type( void ) const { return TT_GAME_TIME; }
		virtual bool Is_Root_Thread( void ) const { return true; }

		virtual void Service( double elapsed_seconds, const CVirtualProcessExecutionContext &context )
		{
			if ( !HasBeenServiced )
			{
				Send_Virtual_Process_Message( MANAGER_THREAD_KEY, shared_ptr< const IVirtualProcessMessage >( new CShutdownVirtualProcessMessage( Get_Key() ) ) );
				HasBeenServiced = true;
			}

			BASECLASS::Service( elapsed_seconds, context );
		}

	private:

		bool HasBeenServiced;
};

TEST_F( ConcurrencyManagerTests, Run_Once_And_Shutdown_Self )
{
	CConcurrencyManagerTester manager_tester;

	SThreadKey test_key( TS_AI, 1, 1 );
	manager_tester.Setup_For_Run( shared_ptr< IManagedVirtualProcess >( new CSuicidalVirtualProcess( test_key ) ) );

	manager_tester.Wait_For_Shutdown();
}

TEST_F( ConcurrencyManagerTests, Stress )
{
	for ( uint32 i = 0; i < 50; ++i )
	{
		CConcurrencyManagerTester manager_tester;

		SThreadKey test_key( TS_AI, 1, 1 );
		manager_tester.Setup_For_Run( shared_ptr< IManagedVirtualProcess >( new CSuicidalVirtualProcess( test_key ) ) );

		manager_tester.Wait_For_Shutdown();
	}
}
