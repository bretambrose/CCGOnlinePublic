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

#include <IPCore/Debug/DebugAssert.h>

#include <IPCore/Memory/Stl/StringStream.h>

#include <iostream>
#include <mutex>
#include <atomic>

static void Build_Assertion_String( const char *expression_string, const char *file_name, uint32_t line_number, bool is_fatal, IP::String &output_string )
{
	IP::StringStream assert_description;
	
	if ( is_fatal )
	{
		assert_description << "** FATAL ASSERT **\n   File: " << file_name << "\n   Line: " << line_number << "\n   Expression: " <<
									 expression_string << "\n Press OK to crash.\n";
	}
	else
	{
		assert_description << "** Assertion Failure **\n   File: " << file_name << "\n   Line: " << line_number << "\n   Expression: " <<
									 expression_string << "\n Press Yes to debug, No to continue.\n";
	}

	output_string = assert_description.rdbuf()->str(); 
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace IP
{
namespace Debug
{
namespace Assert
{

// Static members
static std::mutex AssertLock;
static DLogFunctionType LogFunction;
static std::atomic< bool > Initialized( false );

void Initialize( const DLogFunctionType &log_function )
{
	if ( Initialized )
	{
		return;
	}

	LogFunction = log_function;
	Initialized = true;
}

void Shutdown( void )
{
	if ( Initialized )
	{
		Initialized = false;
	}
}

bool Assert_Handler( const char *expression_string, const char *file_name, uint32_t line_number, bool force_crash )
{
	if ( !Initialized )
	{
		return true;
	}

	std::lock_guard< std::mutex > locker( AssertLock );

	// Build a descriptive error message
	IP::String assert_string;
	::Build_Assertion_String( expression_string, file_name, line_number, force_crash, assert_string );
	
	if ( LogFunction )
	{
		LogFunction( assert_string );
	}

	// bring up a message box
	bool mb_result = false;
	if ( force_crash )
	{
		Modal_Assert_Dialog( "FATAL ASSERT!", assert_string.c_str(), IP::Debug::Assert::AssertDialogType::Ok);
		int32_t *null_dereference = nullptr;
		*null_dereference = 5;
		return false;
	}
	else
	{
		mb_result = Modal_Assert_Dialog( "Assertion Failure!", assert_string.c_str(), IP::Debug::Assert::AssertDialogType::YesNo);
	}

	if ( mb_result )
	{
		Force_Debugger();
	}       

	return true;	
}

} // namespace Assert
} // namespace Debug
} // namespace IP