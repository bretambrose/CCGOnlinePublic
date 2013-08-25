/**********************************************************************************************************************

	TaskSchedulerTests.cpp
		defines unit tests for task scheduler related functionality

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

#include "TaskScheduler/ScheduledTask.h"
#include "TaskScheduler/TaskScheduler.h"

class CMockScheduledTask : public CScheduledTask
{
	public:

		typedef CScheduledTask BASECLASS;

		CMockScheduledTask( double execute_time ) :
			BASECLASS( execute_time ),
			Executed( false )
		{}

		virtual bool Execute( double /*current_time_seconds*/, double & /*reschedule_time_seconds*/ ) 
		{ 
			Executed = true; 
			return false;
		}

		bool Get_Executed( void ) const { return Executed; }

	private:

		bool Executed;
};

TEST( TaskSchedulerTests, Submit )
{
	CTaskScheduler scheduler;

	shared_ptr< CMockScheduledTask > task1( new CMockScheduledTask( 1.0 ) );
	scheduler.Submit_Task( task1 );

	ASSERT_TRUE( task1->Is_Scheduled() );
	ASSERT_TRUE( task1->Get_Heap_Index() == 1 );
	ASSERT_TRUE( scheduler.Get_Next_Task_Time() == 1.0 );

	shared_ptr< CMockScheduledTask > task2( new CMockScheduledTask( 0.5 ) );
	scheduler.Submit_Task( task2 );

	ASSERT_TRUE( task2->Is_Scheduled() );
	ASSERT_TRUE( task1->Get_Heap_Index() == 2 );
	ASSERT_TRUE( task2->Get_Heap_Index() == 1 );
	ASSERT_TRUE( scheduler.Get_Next_Task_Time() == .5 );

	scheduler.Service( .4 );
	ASSERT_TRUE( task1->Is_Scheduled() );
	ASSERT_TRUE( task2->Is_Scheduled() );

	scheduler.Service( .5 );
	ASSERT_TRUE( task1->Is_Scheduled() );
	ASSERT_FALSE( task2->Is_Scheduled() );
	ASSERT_TRUE( task2->Get_Executed() );

	scheduler.Service( 1.1 );
	ASSERT_FALSE( task1->Is_Scheduled() );
	ASSERT_TRUE( task1->Get_Executed() );
}


TEST( TaskSchedulerTests, Remove )
{
	CTaskScheduler scheduler;

	shared_ptr< CMockScheduledTask > tasks[ 5 ];
	for ( uint32 i = 0; i < 5; i++ )
	{
		tasks[ i ].reset( new CMockScheduledTask( static_cast< double >( i + 1 ) ) );
		scheduler.Submit_Task( tasks[ i ] );
	}

	for ( uint32 i = 0; i < 5; i++ )
	{
		ASSERT_TRUE( tasks[ i ]->Is_Scheduled() );
	}

	scheduler.Remove_Task( tasks[ 0 ] );
	ASSERT_FALSE( tasks[ 0 ]->Is_Scheduled() );
	ASSERT_FALSE( tasks[ 0 ]->Get_Executed() );

	scheduler.Remove_Task( tasks[ 3 ] );
	ASSERT_FALSE( tasks[ 3 ]->Is_Scheduled() );
	ASSERT_FALSE( tasks[ 3 ]->Get_Executed() );

	scheduler.Service( 10.0 );
	for ( uint32 i = 0; i < 5; i++ )
	{
		if ( i == 0 || i == 3 )
		{
			ASSERT_FALSE( tasks[ i ]->Get_Executed() );
		}
		else
		{
			ASSERT_TRUE( tasks[ i ]->Get_Executed() );
		}

		ASSERT_FALSE( tasks[ i ]->Is_Scheduled() );
	}
}

TEST( TaskSchedulerTests, Destruction )
{
	CTaskScheduler *scheduler = new CTaskScheduler;

	shared_ptr< CMockScheduledTask > task1( new CMockScheduledTask( 1.0 ) );
	scheduler->Submit_Task( task1 );

	ASSERT_TRUE( task1->Is_Scheduled() );

	delete scheduler;

	ASSERT_FALSE( task1->Is_Scheduled() );
}

TEST( TaskSchedulerTests, Granularity )
{
	CTaskScheduler scheduler( 1.0 );

	shared_ptr< CMockScheduledTask > task1( new CMockScheduledTask( 0.0 ) );
	scheduler.Submit_Task( task1 );
	ASSERT_TRUE( task1->Get_Execute_Time() == 0.0 );

	shared_ptr< CMockScheduledTask > task2( new CMockScheduledTask( 0.01 ) );
	scheduler.Submit_Task( task2 );
	ASSERT_TRUE( task2->Get_Execute_Time() == 1.0 );

	shared_ptr< CMockScheduledTask > task3( new CMockScheduledTask( 0.999 ) );
	scheduler.Submit_Task( task3 );
	ASSERT_TRUE( task3->Get_Execute_Time() == 1.0 );

	shared_ptr< CMockScheduledTask > task4( new CMockScheduledTask( 1.0 ) );
	scheduler.Submit_Task( task4 );
	ASSERT_TRUE( task4->Get_Execute_Time() == 1.0 );

	shared_ptr< CMockScheduledTask > task5( new CMockScheduledTask( 1.1 ) );
	scheduler.Submit_Task( task5 );
	ASSERT_TRUE( task5->Get_Execute_Time() == 2.0 );

	shared_ptr< CMockScheduledTask > task6( new CMockScheduledTask( 1.99 ) );
	scheduler.Submit_Task( task6 );
	ASSERT_TRUE( task6->Get_Execute_Time() == 2.0 );

	shared_ptr< CMockScheduledTask > task7( new CMockScheduledTask( 2.0 ) );
	scheduler.Submit_Task( task7 );
	ASSERT_TRUE( task7->Get_Execute_Time() == 2.0 );

	shared_ptr< CMockScheduledTask > task8( new CMockScheduledTask( 2.5 ) );
	scheduler.Submit_Task( task8 );
	ASSERT_TRUE( task8->Get_Execute_Time() == 3.0 );

	scheduler.Service( 0.0 );
	ASSERT_FALSE( task1->Is_Scheduled() );
	ASSERT_TRUE( task2->Is_Scheduled() );

	scheduler.Service( 1.0 );
	ASSERT_FALSE( task4->Is_Scheduled() );
	ASSERT_TRUE( task5->Is_Scheduled() );

	scheduler.Service( 2.0 );
	ASSERT_FALSE( task7->Is_Scheduled() );
	ASSERT_TRUE( task8->Is_Scheduled() );

	scheduler.Service( 3.0 );
	ASSERT_FALSE( task8->Is_Scheduled() );
}

class CMockRescheduledTask : public CScheduledTask
{
	public:

		typedef CScheduledTask BASECLASS;

		CMockRescheduledTask( double execute_time ) :
			BASECLASS( execute_time ),
			Count( 0 )
		{}

		virtual bool Execute( double current_time_seconds, double &reschedule_time_seconds ) 
		{ 
			++Count;
			reschedule_time_seconds = current_time_seconds + .1;

			return true;
		}

		uint32 Get_Count( void ) const { return Count; }

	private:

		uint32 Count;
};


TEST( TaskSchedulerTests, Reschedule )
{
	CTaskScheduler scheduler;

	shared_ptr< CMockRescheduledTask > task( new CMockRescheduledTask( 1.0 ) );
	scheduler.Submit_Task( task );

	ASSERT_TRUE( task->Get_Count() == 0 );

	scheduler.Service( 0.5 );
	ASSERT_TRUE( task->Get_Count() == 0 );

	scheduler.Service( 1.0 );
	ASSERT_TRUE( task->Get_Count() == 1 );

	scheduler.Service( 2.0 );
	ASSERT_TRUE( task->Get_Count() == 2 );

	scheduler.Service( 2.05 );
	ASSERT_TRUE( task->Get_Count() == 2 );

	scheduler.Service( 2.1 );
	ASSERT_TRUE( task->Get_Count() == 3 );
}