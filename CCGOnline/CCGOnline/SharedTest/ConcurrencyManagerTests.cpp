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
#include "Concurrency/ThreadInterfaces.h"
#include "Concurrency/ThreadStatics.h"
#include "Concurrency/ThreadTaskBase.h"
#include "Concurrency/ThreadSubject.h"
#include "Concurrency/ThreadConstants.h"
#include "Concurrency/ThreadMessageFrame.h"
#include "Concurrency/ThreadMessages/ThreadManagementMessages.h"
#include "Concurrency/ThreadMessages/ExchangeInterfaceMessages.h"
#include "Logging/LogInterface.h"
#include "Time/TimeType.h"
#include "TaskScheduler/ScheduledTask.h"
#include "TaskScheduler/TaskScheduler.h"
#include "PlatformProcess.h"

class ConcurrencyManagerTests : public testing::Test 
{
	protected:  

};

class CThreadTaskBaseExaminer
{
	public:

		CThreadTaskBaseExaminer( const shared_ptr< CThreadTaskBase > &thread_task ) :
			ThreadTask( thread_task )
		{}

		const CThreadTaskBase::FrameTableType &Get_Pending_Outbound_Frames( void ) const { return ThreadTask->PendingOutboundFrames; }
		const CThreadTaskBase::InterfaceTable &Get_Interfaces( void ) const { return ThreadTask->Interfaces; }
		shared_ptr< CReadOnlyThreadInterface > Get_Read_Interface( void ) const { return ThreadTask->ReadInterface; }

		shared_ptr< CWriteOnlyThreadInterface > Get_Log_Interface( void ) const { return ThreadTask->LogInterface; }
		shared_ptr< CWriteOnlyThreadInterface > Get_Manager_Interface( void ) const { return ThreadTask->ManagerInterface; }

	private:

		shared_ptr< CThreadTaskBase > ThreadTask;
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

		void Setup_For_Run( const shared_ptr< IManagerThreadTask > &thread_task )
		{
			Manager->Initialize( false );
			Manager->Setup_For_Run( thread_task );
		}

		void Run_One_Iteration( void )
		{
			Manager->Service_One_Iteration();
			Manager->Set_Game_Time( Manager->Get_Game_Time() + .1 );

			// Wait for all reschedules to return
			while ( !Are_Rescheduled_Threads_Finished() )
			{
				NPlatform::Sleep( 0 );
			}
		}

		void Add_Rescheduled_Thread( const SThreadKey &key ) { RescheduledThreads.insert( key ); }
		void Remove_Rescheduled_Thread( const SThreadKey &key ) { RescheduledThreads.erase( key ); }

		bool Are_Rescheduled_Threads_Finished( void )
		{
			auto thread_set_copy = RescheduledThreads;

			std::vector< shared_ptr< CThreadMessageFrame > > frames;
			shared_ptr< CReadOnlyThreadInterface > read_interface = Manager->Get_Self_Read_Interface();
			read_interface->Remove_Frames( frames );

			// go through all frames, looking for reschedule messages
			for ( uint32 i = 0; i < frames.size(); ++i )
			{
				shared_ptr< CThreadMessageFrame > frame = frames[ i ];
				for ( auto iter = frame->Get_Frame_Begin(); iter != frame->Get_Frame_End(); ++iter )
				{
					const IThreadMessage *raw_message = iter->get();
					if ( Loki::TypeInfo( typeid( *raw_message ) ) == Loki::TypeInfo( typeid( CRescheduleThreadMessage ) ) )
					{
						const CRescheduleThreadMessage *reschedule_message = static_cast< const CRescheduleThreadMessage * >( raw_message );
						if ( thread_set_copy.find( reschedule_message->Get_Key() ) != thread_set_copy.end() )
						{
							thread_set_copy.erase( reschedule_message->Get_Key() );
						}
					}
				}
			}

			// restore all the frames in the manager mailbox
			shared_ptr< CWriteOnlyThreadInterface > write_interface = Manager->Get_Write_Interface( MANAGER_THREAD_KEY );
			for ( uint32 i = 0; i < frames.size(); ++i )
			{
				write_interface->Add_Frame( frames[ i ] );
			}

			return thread_set_copy.size() == 0;
		}

		bool Has_Thread( const SThreadKey &key ) const { return Manager->Get_Record( key ) != nullptr; }
		
		shared_ptr< IManagerThreadTask > Get_Thread_Task( const SThreadKey &key ) const { return Manager->Get_Thread_Task( key ); }

		void Shutdown( void )
		{
			if ( Manager->ThreadRecords.size() > 0 )
			{
				shared_ptr< CThreadMessageFrame > frame( new CThreadMessageFrame( MANAGER_THREAD_KEY ) );
				frame->Add_Message( shared_ptr< const IThreadMessage >( new CShutdownManagerMessage ) );

				shared_ptr< CWriteOnlyThreadInterface > write_interface = Manager->Get_Write_Interface( MANAGER_THREAD_KEY );
				write_interface->Add_Frame( frame );

				Wait_For_Shutdown();
			}
		}

		void Wait_For_Shutdown( void )
		{
			while ( Manager->ThreadRecords.size() > 0 )
			{
				Manager->Set_Game_Time( Manager->Get_Game_Time() + .1 );
				Manager->Service_One_Iteration();

				NPlatform::Sleep( 0 );
			}

			RescheduledThreads.clear();
		}

		const CConcurrencyManager::FrameTableType &Get_Pending_Outbound_Frames( void ) const { return Manager->PendingOutboundFrames; }

		shared_ptr< CWriteOnlyThreadInterface > Get_Write_Interface( void ) const { return Manager->Get_Write_Interface( MANAGER_THREAD_KEY ); }

		size_t Get_Pending_Interface_Request_Count( void ) const { return Manager->UnfulfilledPushRequests.size() + Manager->UnfulfilledGetRequests.size(); }

	private:

		scoped_ptr< CConcurrencyManager > Manager;

		std::set< SThreadKey, SThreadKeyContainerHelper > RescheduledThreads;
};

class CDoNothingThreadTask : public CThreadTaskBase
{
	public:

		typedef CThreadTaskBase BASECLASS;

		CDoNothingThreadTask( const SThreadKey &key ) :
			BASECLASS( key ),
			ServiceCount( 0 )
		{}

		virtual ETimeType Get_Time_Type( void ) const { return TT_GAME_TIME; }
		virtual bool Is_Root_Thread( void ) const { return true; }

		virtual void Service( double elapsed_seconds, const CThreadTaskExecutionContext &context )
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
	manager_tester.Setup_For_Run( shared_ptr< IManagerThreadTask >( new CDoNothingThreadTask( test_key ) ) );

	manager_tester.Add_Rescheduled_Thread( test_key );

	// Verify setup state
	// 3 thread records: manager, log, test_key
	ASSERT_TRUE( manager_tester.Has_Thread( test_key ) );
	ASSERT_TRUE( manager_tester.Has_Thread( MANAGER_THREAD_KEY ) );
	ASSERT_TRUE( manager_tester.Has_Thread( LOG_THREAD_KEY ) );

	// 1 push req: log
	auto outbound_frames = manager_tester.Get_Pending_Outbound_Frames();
	ASSERT_TRUE( outbound_frames.size() == 1 );
	ASSERT_TRUE( outbound_frames.find( test_key ) != outbound_frames.end() );

	shared_ptr< CThreadMessageFrame > frame = outbound_frames[ test_key ];
	for ( auto frame_iter = frame->Get_Frame_Begin(); frame_iter != frame->Get_Frame_End(); ++frame_iter )
	{
		const IThreadMessage *raw_message = frame_iter->get();
		ASSERT_TRUE( Loki::TypeInfo( typeid( *raw_message ) ) == Loki::TypeInfo( typeid( CAddInterfaceMessage ) ) );
	}

	// Run a single frame
	manager_tester.Run_One_Iteration();
	
	// Verify post-run state
	// thread should have been run once and have a log interface
	CThreadTaskBaseExaminer test_thread( static_pointer_cast< CThreadTaskBase >( manager_tester.Get_Thread_Task( test_key ) ) );
	auto interfaces = test_thread.Get_Interfaces();
	ASSERT_TRUE( interfaces.size() == 0 );
	ASSERT_TRUE( test_thread.Get_Log_Interface().get() != nullptr );

	shared_ptr< CDoNothingThreadTask > do_nothing_task = static_pointer_cast< CDoNothingThreadTask >( manager_tester.Get_Thread_Task( test_key ) );
	ASSERT_TRUE( do_nothing_task->Get_Service_Count() == 1 );

	manager_tester.Shutdown();
}



static const SThreadKey PUSH_GET_KEY1( TS_AI, 1, 1 );
static const SThreadKey PUSH_GET_KEY2( TS_AI, 1, 2 );
static const SThreadKey PUSH_GET_KEY3( TS_AI, 1, 3 );
static const SThreadKey PUSH_GET_KEY4( TS_AI, 2, 1 );
static const SThreadKey PUSH_GET_KEY5( TS_UI, 1, 1 );

class CInterfaceTestThreadTask : public CDoNothingThreadTask
{
	public:

		typedef CDoNothingThreadTask BASECLASS;

		CInterfaceTestThreadTask( const SThreadKey &key ) :
			BASECLASS( key ),
			HasBeenServiced( false )
		{}

		virtual ETimeType Get_Time_Type( void ) const { return TT_GAME_TIME; }
		virtual bool Is_Root_Thread( void ) const { return true; }

		virtual void Service( double elapsed_seconds, const CThreadTaskExecutionContext &context )
		{
			if ( !HasBeenServiced )
			{
				HasBeenServiced = true;

				const SThreadKey &key = Get_Key();
				if ( key == PUSH_GET_KEY1 )
				{
					Send_Thread_Message( MANAGER_THREAD_KEY, shared_ptr< const IThreadMessage >( new CGetInterfaceRequest( key, PUSH_GET_KEY4 ) ) );
				}
				else if ( key == PUSH_GET_KEY2 )
				{
					Send_Thread_Message( MANAGER_THREAD_KEY, shared_ptr< const IThreadMessage >( new CGetInterfaceRequest( key, PUSH_GET_KEY4 ) ) );
				}
				else if ( key == PUSH_GET_KEY3 )
				{
					Send_Thread_Message( MANAGER_THREAD_KEY, shared_ptr< const IThreadMessage >( new CGetInterfaceRequest( key, PUSH_GET_KEY4 ) ) );
				}
				else if ( key == PUSH_GET_KEY4 )
				{
					Send_Thread_Message( MANAGER_THREAD_KEY, shared_ptr< const IThreadMessage >( new CPushInterfaceRequest( key, PUSH_GET_KEY5 ) ) );
				}
				else if ( key == PUSH_GET_KEY5 )
				{
					Send_Thread_Message( MANAGER_THREAD_KEY, shared_ptr< const IThreadMessage >( new CPushInterfaceRequest( key, SThreadKey( TS_AI, MAJOR_KEY_ALL, MINOR_KEY_ALL ) ) ) );
					Send_Thread_Message( MANAGER_THREAD_KEY, shared_ptr< const IThreadMessage >( new CGetInterfaceRequest( key, SThreadKey( TS_AI, 1, MINOR_KEY_ALL ) ) ) );
				}
			}

			BASECLASS::Service( elapsed_seconds, context );
		}

	private:

		bool HasBeenServiced;
};

class CSpawnInterfacePushGetThreadTask : public CThreadTaskBase
{
	public:

		typedef CThreadTaskBase BASECLASS;

		CSpawnInterfacePushGetThreadTask( const SThreadKey &key ) :
			BASECLASS( key ),
			HasBeenServiced( false )
		{}

		virtual ETimeType Get_Time_Type( void ) const { return TT_GAME_TIME; }
		virtual bool Is_Root_Thread( void ) const { return true; }

		virtual void Service( double elapsed_seconds, const CThreadTaskExecutionContext &context )
		{
			if ( !HasBeenServiced )
			{
				CThreadStatics::Get_Current_Thread_Task()->Send_Thread_Message( MANAGER_THREAD_KEY, 
																									 shared_ptr< const IThreadMessage >( new CAddThreadMessage( 
																											shared_ptr< IThreadTask >( new CInterfaceTestThreadTask( PUSH_GET_KEY1 ) ), false, false ) ) );

				CThreadStatics::Get_Current_Thread_Task()->Send_Thread_Message( MANAGER_THREAD_KEY, 
																									 shared_ptr< const IThreadMessage >( new CAddThreadMessage( 
																											shared_ptr< IThreadTask >( new CInterfaceTestThreadTask( PUSH_GET_KEY2 ) ), false, false ) ) );

				CThreadStatics::Get_Current_Thread_Task()->Send_Thread_Message( MANAGER_THREAD_KEY, 
																									 shared_ptr< const IThreadMessage >( new CAddThreadMessage( 
																											shared_ptr< IThreadTask >( new CInterfaceTestThreadTask( PUSH_GET_KEY3 ) ), false, false ) ) );

				CThreadStatics::Get_Current_Thread_Task()->Send_Thread_Message( MANAGER_THREAD_KEY, 
																									 shared_ptr< const IThreadMessage >( new CAddThreadMessage( 
																											shared_ptr< IThreadTask >( new CInterfaceTestThreadTask( PUSH_GET_KEY4 ) ), false, false ) ) );

				CThreadStatics::Get_Current_Thread_Task()->Send_Thread_Message( MANAGER_THREAD_KEY, 
																									 shared_ptr< const IThreadMessage >( new CAddThreadMessage( 
																											shared_ptr< IThreadTask >( new CInterfaceTestThreadTask( PUSH_GET_KEY5 ) ), false, false ) ) );

				HasBeenServiced = true;
			}

			BASECLASS::Service( elapsed_seconds, context );
		}

	private:

		bool HasBeenServiced;
};

void Verify_Interfaces_Present( const CConcurrencyManagerTester &manager_tester )
{
	CThreadTaskBaseExaminer test_thread1( static_pointer_cast< CThreadTaskBase >( manager_tester.Get_Thread_Task( PUSH_GET_KEY1 ) ) );
	auto interfaces = test_thread1.Get_Interfaces();
	ASSERT_TRUE( interfaces.size() == 2 );
	ASSERT_TRUE( interfaces.find( PUSH_GET_KEY4 ) != interfaces.end() );
	ASSERT_TRUE( interfaces.find( PUSH_GET_KEY5 ) != interfaces.end() );
	ASSERT_TRUE( test_thread1.Get_Log_Interface().get() != nullptr );
	ASSERT_TRUE( test_thread1.Get_Manager_Interface().get() != nullptr );

	CThreadTaskBaseExaminer test_thread2( static_pointer_cast< CThreadTaskBase >( manager_tester.Get_Thread_Task( PUSH_GET_KEY2 ) ) );
	interfaces = test_thread2.Get_Interfaces();
	ASSERT_TRUE( interfaces.size() == 2 );
	ASSERT_TRUE( interfaces.find( PUSH_GET_KEY4 ) != interfaces.end() );
	ASSERT_TRUE( interfaces.find( PUSH_GET_KEY5 ) != interfaces.end() );
	ASSERT_TRUE( test_thread2.Get_Log_Interface().get() != nullptr );
	ASSERT_TRUE( test_thread2.Get_Manager_Interface().get() != nullptr );

	CThreadTaskBaseExaminer test_thread3( static_pointer_cast< CThreadTaskBase >( manager_tester.Get_Thread_Task( PUSH_GET_KEY3 ) ) );
	interfaces = test_thread3.Get_Interfaces();
	ASSERT_TRUE( interfaces.size() == 2 );
	ASSERT_TRUE( interfaces.find( PUSH_GET_KEY4 ) != interfaces.end() );
	ASSERT_TRUE( interfaces.find( PUSH_GET_KEY5 ) != interfaces.end() );
	ASSERT_TRUE( test_thread3.Get_Log_Interface().get() != nullptr );
	ASSERT_TRUE( test_thread3.Get_Manager_Interface().get() != nullptr );

	CThreadTaskBaseExaminer test_thread4( static_pointer_cast< CThreadTaskBase >( manager_tester.Get_Thread_Task( PUSH_GET_KEY4 ) ) );
	interfaces = test_thread4.Get_Interfaces();
	ASSERT_TRUE( interfaces.size() == 1 );
	ASSERT_TRUE( interfaces.find( PUSH_GET_KEY5 ) != interfaces.end() );
	ASSERT_TRUE( test_thread4.Get_Log_Interface().get() != nullptr );
	ASSERT_TRUE( test_thread4.Get_Manager_Interface().get() != nullptr );

	CThreadTaskBaseExaminer test_thread5( static_pointer_cast< CThreadTaskBase >( manager_tester.Get_Thread_Task( PUSH_GET_KEY5 ) ) );
	interfaces = test_thread5.Get_Interfaces();
	ASSERT_TRUE( interfaces.size() == 4 );
	ASSERT_TRUE( interfaces.find( PUSH_GET_KEY1 ) != interfaces.end() );
	ASSERT_TRUE( interfaces.find( PUSH_GET_KEY2 ) != interfaces.end() );
	ASSERT_TRUE( interfaces.find( PUSH_GET_KEY3 ) != interfaces.end() );
	ASSERT_TRUE( interfaces.find( PUSH_GET_KEY4 ) != interfaces.end() );
	ASSERT_TRUE( test_thread5.Get_Log_Interface().get() != nullptr );
	ASSERT_TRUE( test_thread5.Get_Manager_Interface().get() != nullptr );
}

TEST_F( ConcurrencyManagerTests, Interface_Push_Get1 )
{
	CConcurrencyManagerTester manager_tester;

	SThreadKey test_key( TS_LOGIC, 1, 1 );
	manager_tester.Setup_For_Run( shared_ptr< IManagerThreadTask >( new CSpawnInterfacePushGetThreadTask( test_key ) ) );

	manager_tester.Add_Rescheduled_Thread( test_key );
	manager_tester.Run_One_Iteration();
	manager_tester.Run_One_Iteration();

	ASSERT_TRUE( manager_tester.Has_Thread( test_key ) );
	ASSERT_TRUE( manager_tester.Has_Thread( PUSH_GET_KEY1 ) );
	ASSERT_TRUE( manager_tester.Has_Thread( PUSH_GET_KEY2 ) );
	ASSERT_TRUE( manager_tester.Has_Thread( PUSH_GET_KEY3 ) );
	ASSERT_TRUE( manager_tester.Has_Thread( PUSH_GET_KEY4 ) );
	ASSERT_TRUE( manager_tester.Has_Thread( PUSH_GET_KEY5 ) );

	manager_tester.Add_Rescheduled_Thread( PUSH_GET_KEY1 );
	manager_tester.Add_Rescheduled_Thread( PUSH_GET_KEY2 );
	manager_tester.Add_Rescheduled_Thread( PUSH_GET_KEY3 );
	manager_tester.Add_Rescheduled_Thread( PUSH_GET_KEY4 );
	manager_tester.Add_Rescheduled_Thread( PUSH_GET_KEY5 );
	manager_tester.Run_One_Iteration();

	manager_tester.Run_One_Iteration();

	Verify_Interfaces_Present( manager_tester );

	ASSERT_TRUE( manager_tester.Get_Pending_Interface_Request_Count() == 0 );

	manager_tester.Shutdown();
}

class CSpawnInterfacePushGetThreadTask2 : public CThreadTaskBase
{
	public:

		typedef CThreadTaskBase BASECLASS;

		CSpawnInterfacePushGetThreadTask2( const SThreadKey &key ) :
			BASECLASS( key ),
			HasBeenServiced( false )
		{}

		virtual ETimeType Get_Time_Type( void ) const { return TT_GAME_TIME; }
		virtual bool Is_Root_Thread( void ) const { return true; }

		virtual void Service( double elapsed_seconds, const CThreadTaskExecutionContext &context )
		{
			if ( !HasBeenServiced )
			{
				CThreadStatics::Get_Current_Thread_Task()->Send_Thread_Message( MANAGER_THREAD_KEY, 
																									 shared_ptr< const IThreadMessage >( new CAddThreadMessage( 
																											shared_ptr< IThreadTask >( new CDoNothingThreadTask( PUSH_GET_KEY1 ) ), false, false ) ) );

				CThreadStatics::Get_Current_Thread_Task()->Send_Thread_Message( MANAGER_THREAD_KEY, 
																									 shared_ptr< const IThreadMessage >( new CAddThreadMessage( 
																											shared_ptr< IThreadTask >( new CDoNothingThreadTask( PUSH_GET_KEY2 ) ), false, false ) ) );

				CThreadStatics::Get_Current_Thread_Task()->Send_Thread_Message( MANAGER_THREAD_KEY, 
																									 shared_ptr< const IThreadMessage >( new CAddThreadMessage( 
																											shared_ptr< IThreadTask >( new CDoNothingThreadTask( PUSH_GET_KEY3 ) ), false, false ) ) );

				CThreadStatics::Get_Current_Thread_Task()->Send_Thread_Message( MANAGER_THREAD_KEY, 
																									 shared_ptr< const IThreadMessage >( new CAddThreadMessage( 
																											shared_ptr< IThreadTask >( new CDoNothingThreadTask( PUSH_GET_KEY4 ) ), false, false ) ) );

				CThreadStatics::Get_Current_Thread_Task()->Send_Thread_Message( MANAGER_THREAD_KEY, 
																									 shared_ptr< const IThreadMessage >( new CAddThreadMessage( 
																											shared_ptr< IThreadTask >( new CDoNothingThreadTask( PUSH_GET_KEY5 ) ), false, false ) ) );

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
	manager_tester.Setup_For_Run( shared_ptr< IManagerThreadTask >( new CSpawnInterfacePushGetThreadTask2( test_key ) ) );

	shared_ptr< CThreadMessageFrame > frame( new CThreadMessageFrame( MANAGER_THREAD_KEY ) );

	frame->Add_Message( shared_ptr< const IThreadMessage >( new CGetInterfaceRequest( PUSH_GET_KEY1, PUSH_GET_KEY4 ) ) );
	frame->Add_Message( shared_ptr< const IThreadMessage >( new CGetInterfaceRequest( PUSH_GET_KEY2, PUSH_GET_KEY4 ) ) );
	frame->Add_Message( shared_ptr< const IThreadMessage >( new CGetInterfaceRequest( PUSH_GET_KEY3, PUSH_GET_KEY4 ) ) );
	frame->Add_Message( shared_ptr< const IThreadMessage >( new CPushInterfaceRequest( PUSH_GET_KEY4, PUSH_GET_KEY5 ) ) );
	frame->Add_Message( shared_ptr< const IThreadMessage >( new CPushInterfaceRequest( PUSH_GET_KEY5, SThreadKey( TS_AI, MAJOR_KEY_ALL, MINOR_KEY_ALL ) ) ) );
	frame->Add_Message( shared_ptr< const IThreadMessage >( new CGetInterfaceRequest( PUSH_GET_KEY5, SThreadKey( TS_AI, 1, MINOR_KEY_ALL ) ) ) );

	auto write_interface = manager_tester.Get_Write_Interface();
	write_interface->Add_Frame( frame );

	manager_tester.Add_Rescheduled_Thread( test_key );
	manager_tester.Run_One_Iteration();

	ASSERT_TRUE( manager_tester.Get_Pending_Interface_Request_Count() == 4 );

	manager_tester.Run_One_Iteration();

	ASSERT_TRUE( manager_tester.Has_Thread( test_key ) );
	ASSERT_TRUE( manager_tester.Has_Thread( PUSH_GET_KEY1 ) );
	ASSERT_TRUE( manager_tester.Has_Thread( PUSH_GET_KEY2 ) );
	ASSERT_TRUE( manager_tester.Has_Thread( PUSH_GET_KEY3 ) );
	ASSERT_TRUE( manager_tester.Has_Thread( PUSH_GET_KEY4 ) );
	ASSERT_TRUE( manager_tester.Has_Thread( PUSH_GET_KEY5 ) );

	manager_tester.Add_Rescheduled_Thread( PUSH_GET_KEY1 );
	manager_tester.Add_Rescheduled_Thread( PUSH_GET_KEY2 );
	manager_tester.Add_Rescheduled_Thread( PUSH_GET_KEY3 );
	manager_tester.Add_Rescheduled_Thread( PUSH_GET_KEY4 );
	manager_tester.Add_Rescheduled_Thread( PUSH_GET_KEY5 );
	manager_tester.Run_One_Iteration();

	manager_tester.Run_One_Iteration();

	Verify_Interfaces_Present( manager_tester );

	ASSERT_TRUE( manager_tester.Get_Pending_Interface_Request_Count() == 0 );

	manager_tester.Shutdown();
}

class CSuicidalThreadTask : public CThreadTaskBase
{
	public:

		typedef CThreadTaskBase BASECLASS;

		CSuicidalThreadTask( const SThreadKey &key ) :
			BASECLASS( key ),
			HasBeenServiced( false )
		{}

		virtual ETimeType Get_Time_Type( void ) const { return TT_GAME_TIME; }
		virtual bool Is_Root_Thread( void ) const { return true; }

		virtual void Service( double elapsed_seconds, const CThreadTaskExecutionContext &context )
		{
			if ( !HasBeenServiced )
			{
				Send_Thread_Message( MANAGER_THREAD_KEY, shared_ptr< const IThreadMessage >( new CShutdownThreadMessage( Get_Key() ) ) );
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
	manager_tester.Setup_For_Run( shared_ptr< IManagerThreadTask >( new CSuicidalThreadTask( test_key ) ) );

	manager_tester.Wait_For_Shutdown();
}

TEST_F( ConcurrencyManagerTests, Stress )
{
	for ( uint32 i = 0; i < 50; ++i )
	{
		CConcurrencyManagerTester manager_tester;

		SThreadKey test_key( TS_AI, 1, 1 );
		manager_tester.Setup_For_Run( shared_ptr< IManagerThreadTask >( new CSuicidalThreadTask( test_key ) ) );

		manager_tester.Wait_For_Shutdown();
	}
}
