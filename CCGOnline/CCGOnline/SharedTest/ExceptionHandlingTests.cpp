/**********************************************************************************************************************

	[Placeholder for eventual source license]

	ExceptionHandlingTests.cpp
		defines unit tests for exception handling functionality

	(c) Copyright 2011, Bret Ambrose.  All rights reserved.

**********************************************************************************************************************/

#include "stdafx.h"

#include "StructuredExceptionHandler.h"
#include "StructuredExceptionInfo.h"
#include "WindowsWrapper.h"
#include "PlatformProcess.h"

void Fake_Exception_Handler( CStructuredExceptionInfo &info )
{
	ASSERT_TRUE( info.Get_Exception_Message() == L"Unknown Exception Code: 1" );

#ifdef _DEBUG
	// the call stack changes between 32 and 64 bit, as well as debug and release
	const std::vector< CStackFrame > &call_stack = info.Get_Call_Stack();

	ASSERT_TRUE( call_stack[ 0 ].Get_Function_Name() == L"RaiseException" );
	ASSERT_TRUE( call_stack[ 0 ].Get_File_Name().size() == 0 );
	ASSERT_TRUE( call_stack[ 1 ].Get_Function_Name() == L"Exception_Level_3" );
	ASSERT_TRUE( call_stack[ 1 ].Get_Module_Name() == NPlatform::Get_Exe_Name() );
	ASSERT_TRUE( call_stack[ 1 ].Get_Line_Number() > 0 );
#endif // _DEBUG
}

class ExceptionHandlingTests : public testing::Test 
{
	protected:  

		virtual void SetUp( void )
		{
			CStructuredExceptionHandler::Shutdown();
			CStructuredExceptionHandler::Initialize( DExceptionHandler( Fake_Exception_Handler ) );
		}

		virtual void TearDown( void )
		{
			CStructuredExceptionHandler::Shutdown();
			CStructuredExceptionHandler::Initialize();
		}
};

void Exception_Level_3( void )
{
	RaiseException( 1, 0, 0, NULL );
}

void Exception_Level_2( void )
{
	Exception_Level_3();
}

void Exception_Level_1( void )
{
	Exception_Level_2();
}

TEST_F( ExceptionHandlingTests, FakeException )
{
	Exception_Level_1();
}
