/**********************************************************************************************************************

	DebugAssert.cpp
		Implementation of a static class to catch and log asserts with.

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

#include "DebugAssert.h"

#include <sstream>
#include <iostream>

#include "SynchronizationPrimitives/PlatformMutex.h"

/**********************************************************************************************************************
	CAssertSystem::Build_Assertion_String -- builds a descriptive error string from information supplied by the assert
	
		expression_string -- the conditional expression that failed
		file_name -- file where this check was located
		line_number -- line number where this check was located
		is_fatal -- is this assertion a fatal one?
		output_string -- output parameter for the descriptive error string
		
**********************************************************************************************************************/
static void Build_Assertion_String( const char *expression_string, const char *file_name, uint32 line_number, bool is_fatal, std::wstring &output_string )
{
	std::basic_ostringstream< wchar_t > assert_description;
	
	if ( is_fatal )
	{
		assert_description << L"** FATAL ASSERT **\n   File: " << file_name << L"\n   Line: " << line_number << L"\n   Expression: " <<
									 expression_string << L"\n Press OK to crash.\n";
	}
	else
	{
		assert_description << L"** Assertion Failure **\n   File: " << file_name << L"\n   Line: " << line_number << L"\n   Expression: " <<
									 expression_string << L"\n Press Yes to debug, No to continue.\n";
	}

	output_string = assert_description.rdbuf()->str(); 
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// Static members
ISimplePlatformMutex *CAssertSystem::AssertLock = nullptr;
DLogFunctionType CAssertSystem::LogFunction;
bool CAssertSystem::Initialized = false;

/**********************************************************************************************************************
	CAssertSystem::Initialize -- initializes the assert handling system

		log_function -- a callback that will log the assert

**********************************************************************************************************************/
void CAssertSystem::Initialize( const DLogFunctionType &log_function )
{
	FATAL_ASSERT( !Initialized );

	LogFunction = log_function;
	AssertLock = NPlatform::Create_Simple_Mutex();
	Initialized = true;
}

/**********************************************************************************************************************
	CAssertSystem::Shutdown -- shuts down and cleans up the assert handling system

**********************************************************************************************************************/
void CAssertSystem::Shutdown( void )
{
	if ( Initialized )
	{
		delete AssertLock;
		AssertLock = nullptr;
		Initialized = false;
	}
}

/**********************************************************************************************************************
	CAssertSystem::Assert_Handler -- handles an assert by building a descriptive message and displaying a dialog
		to the user with the option of debugging/continuing if this is a non-fatal assert
	
		expression_string -- the conditional expression that failed
		file_name -- file where this check was located
		line_number -- line number where this check was located
		force_crash -- is this a fatal assert?

		Returns: always returns true in order to support FAIL_IF conditional evaluation
		
********************************************************************************************/
bool CAssertSystem::Assert_Handler( const char *expression_string, const char *file_name, uint32 line_number, bool force_crash )
{
	if ( !Initialized )
	{
		return true;
	}

	CSimplePlatformMutexLocker assert_locker( AssertLock );

	// Build a descriptive error message
	std::wstring assert_string;
	::Build_Assertion_String( expression_string, file_name, line_number, force_crash, assert_string );
	
	if ( !LogFunction.empty() )
	{
		LogFunction( assert_string );
	}

	// bring up a message box
	int result = 0;
	if ( force_crash )
	{
		::MessageBox( NULL, assert_string.c_str(), L"FATAL ASSERT!", MB_OK | MB_ICONEXCLAMATION | MB_SETFOREGROUND | MB_TASKMODAL | MB_DEFBUTTON3 );
		int32 *null_dereference = nullptr;
		*null_dereference = 5;
		return false;
	}
	else
	{
		result = ::MessageBox( NULL, assert_string.c_str(), L"Assertion Failure!", MB_YESNO | MB_ICONEXCLAMATION | MB_SETFOREGROUND | MB_TASKMODAL | MB_DEFBUTTON3 );
	}

	if ( result == IDYES )
	{
		DebugBreak();
	}       

	return true;	
}

