/**********************************************************************************************************************

	[Placeholder for eventual source license]

	PlatformExceptionHandler.h
		A component that wraps OS-specific exception handling logic, including generating a stack trace

	(c) Copyright 2011, Bret Ambrose.  All rights reserved.

**********************************************************************************************************************/

#ifndef PLATFORM_EXCEPTION_HANDLER_H
#define PLATFORM_EXCEPTION_HANDLER_H

#include "StructuredExceptionTypes.h"

class CStructuredExceptionInfo;
class ISimplePlatformMutex;

// Static class that performs OS-specific structured exception handling logic
class CPlatformExceptionHandler
{
	public:

		static void Initialize( const DExceptionHandler &handler );
		static void Shutdown( void );

		static bool Load_Symbols( std::wstring &error_message );

		static void On_Exception( CStructuredExceptionInfo &shared_exception_info );

		static ISimplePlatformMutex *Get_Lock( void ) { return ExceptionLock; }

	private:

		static DExceptionHandler Handler;

		static ISimplePlatformMutex *ExceptionLock;

		static bool SymbolsLoaded;
		static bool Initialized;
};

#endif // PLATFORM_EXCEPTION_HANDLER_H
