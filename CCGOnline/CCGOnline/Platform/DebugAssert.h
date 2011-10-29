/**********************************************************************************************************************

	[Placeholder for eventual source license]

	DebugAssert.h
		A static class to catch and log asserts with, together with a set of assert macros

	(c) Copyright 2011, Bret Ambrose.  All rights reserved.

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
