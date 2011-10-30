/**********************************************************************************************************************

	DebugAssert.h
		A static class to catch and log asserts with, together with a set of assert macros

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

#ifndef DEBUG_ASSERT_H
#define DEBUG_ASSERT_H

class ISimplePlatformMutex;

typedef FastDelegate1< const std::wstring &, void > DLogFunctionType;

// A static class to handle all assertion failures.  Multi-threaded safe via a mutex.
class CAssertSystem
{
	public:

		static void Initialize( const DLogFunctionType &log_function );
		static void Shutdown( void );

		// all forms of condition checking go here on failure
		static bool Assert_Handler( const char *expression_string, const char *file_name, uint32 line_number, bool force_crash );

	private:

		static ISimplePlatformMutex *AssertLock;

		static DLogFunctionType LogFunction;

		static bool Initialized;
};

// Macros
#ifdef _DEBUG

#define FAIL_IF( x ) if ( ( x ) && CAssertSystem::Assert_Handler( #x, __FILE__, __LINE__, false ) )
#define DEBUG_ASSERT( x ) ( ( x ) || CAssertSystem::Assert_Handler( #x, __FILE__, __LINE__, false ) )

#else

#define FAIL_IF( x ) if ( x )
#define DEBUG_ASSERT( x )

#endif // _DEBUG

#define FATAL_ASSERT( x ) ( ( x ) || CAssertSystem::Assert_Handler( #x, __FILE__, __LINE__, true ) )

#endif // DEBUG_ASSERT_H
