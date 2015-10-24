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

#include "stdafx.h"

#include "DebugAssert.h"

#include <sstream>
#include <iostream>

static void Build_Assertion_String( const char *expression_string, const char *file_name, uint32_t line_number, bool is_fatal, std::wstring &output_string )
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

namespace IP
{
namespace Debug
{

// Static members
std::mutex CAssertSystem::AssertLock;
DLogFunctionType CAssertSystem::LogFunction;
std::atomic< bool > CAssertSystem::Initialized( false );

void CAssertSystem::Initialize( const DLogFunctionType &log_function )
{
	FATAL_ASSERT( !Initialized );

	LogFunction = log_function;
	Initialized = true;
}

void CAssertSystem::Shutdown( void )
{
	if ( Initialized )
	{
		Initialized = false;
	}
}

bool CAssertSystem::Assert_Handler( const char *expression_string, const char *file_name, uint32_t line_number, bool force_crash )
{
	if ( !Initialized )
	{
		return true;
	}

	std::lock_guard<std::mutex> locker( AssertLock );

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
		::MessageBox( nullptr, assert_string.c_str(), L"FATAL ASSERT!", MB_OK | MB_ICONEXCLAMATION | MB_SETFOREGROUND | MB_TASKMODAL | MB_DEFBUTTON3 );
		int32_t *null_dereference = nullptr;
		*null_dereference = 5;
		return false;
	}
	else
	{
		result = ::MessageBox( nullptr, assert_string.c_str(), L"Assertion Failure!", MB_YESNO | MB_ICONEXCLAMATION | MB_SETFOREGROUND | MB_TASKMODAL | MB_DEFBUTTON3 );
	}

	if ( result == IDYES )
	{
		DebugBreak();
	}       

	return true;	
}

} // namespace Debug
} // namespace IP