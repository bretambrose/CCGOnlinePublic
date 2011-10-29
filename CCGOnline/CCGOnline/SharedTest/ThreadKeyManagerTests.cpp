/**********************************************************************************************************************

	[Placeholder for eventual source license]

	ThreadKeyManagerTests.cpp
		defines unit tests for thread key manager related functionality

	(c) Copyright 2011, Bret Ambrose.  All rights reserved.

**********************************************************************************************************************/

#include "stdafx.h"

#include "Concurrency/ThreadKeyManager.h"
#include "Concurrency/ThreadKey.h"
#include "Concurrency/ThreadSubject.h"

TEST( ThreadKeyManagerTests, Add_Remove_Fill_In )
{
	CThreadKeyManager key_manager;

	SThreadKey test_key = key_manager.Fill_In_Thread_Key( SThreadKey( TS_UI, 0, 0 ) );
	ASSERT_TRUE( test_key == SThreadKey( TS_UI, 1, 1 ) );

	test_key = key_manager.Fill_In_Thread_Key( SThreadKey( TS_UI, 2, 0 ) );
	ASSERT_TRUE( test_key == SThreadKey( TS_UI, 2, 1 ) );

	test_key = key_manager.Fill_In_Thread_Key( SThreadKey( TS_UI, 0, 2 ) );
	ASSERT_TRUE( test_key == SThreadKey( TS_UI, 1, 2 ) );

	// Minor key allocation
	key_manager.Add_Tracked_Thread_Key( SThreadKey( TS_UI, 1, 1 ) );
	ASSERT_TRUE( key_manager.Fill_In_Thread_Key( SThreadKey( TS_UI, 1, 0 ) ) == SThreadKey( TS_UI, 1, 2 ) );

	key_manager.Add_Tracked_Thread_Key( SThreadKey( TS_UI, 1, 2 ) );
	ASSERT_TRUE( key_manager.Fill_In_Thread_Key( SThreadKey( TS_UI, 1, 0 ) ) == SThreadKey( TS_UI, 1, 3 ) );

	key_manager.Remove_Tracked_Thread_Key( SThreadKey( TS_UI, 1, 2 ) );
	ASSERT_TRUE( key_manager.Fill_In_Thread_Key( SThreadKey( TS_UI, 1, 0 ) ) == SThreadKey( TS_UI, 1, 2 ) );

	key_manager.Remove_Tracked_Thread_Key( SThreadKey( TS_UI, 1, 1 ) );
	ASSERT_TRUE( key_manager.Fill_In_Thread_Key( SThreadKey( TS_UI, 1, 0 ) ) == SThreadKey( TS_UI, 1, 1 ) );

	// Major key allocation
	key_manager.Add_Tracked_Thread_Key( SThreadKey( TS_UI, 1, 1 ) );
	ASSERT_TRUE( key_manager.Fill_In_Thread_Key( SThreadKey( TS_UI, 0, 1 ) ) == SThreadKey( TS_UI, 2, 1 ) );

	key_manager.Add_Tracked_Thread_Key( SThreadKey( TS_UI, 2, 1 ) );
	ASSERT_TRUE( key_manager.Fill_In_Thread_Key( SThreadKey( TS_UI, 0, 1 ) ) == SThreadKey( TS_UI, 3, 1 ) );

	key_manager.Remove_Tracked_Thread_Key( SThreadKey( TS_UI, 1, 1 ) );
	ASSERT_TRUE( key_manager.Fill_In_Thread_Key( SThreadKey( TS_UI, 0, 1 ) ) == SThreadKey( TS_UI, 1, 1 ) );

	key_manager.Remove_Tracked_Thread_Key( SThreadKey( TS_UI, 2, 1 ) );
	ASSERT_TRUE( key_manager.Fill_In_Thread_Key( SThreadKey( TS_UI, 0, 1 ) ) == SThreadKey( TS_UI, 1, 1 ) );

	// Finding a "hole" allocation
	key_manager.Add_Tracked_Thread_Key( SThreadKey( TS_UI, 1, 1 ) );
	key_manager.Add_Tracked_Thread_Key( SThreadKey( TS_UI, 1, 2 ) );
	key_manager.Add_Tracked_Thread_Key( SThreadKey( TS_UI, 1, 4 ) );
	ASSERT_TRUE( key_manager.Fill_In_Thread_Key( SThreadKey( TS_UI, 1, 0 ) ) == SThreadKey( TS_UI, 1, 3 ) );

}