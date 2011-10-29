/**********************************************************************************************************************

	[Placeholder for eventual source license]

	StructuredExceptionHandler.h
		A component that does platform-agnostic exception handling

	(c) Copyright 2011, Bret Ambrose.  All rights reserved.

**********************************************************************************************************************/

#ifndef STRUCTURED_EXCEPTION_HANDLER_H
#define STRUCTURED_EXCEPTION_HANDLER_H

#include "StructuredExceptionTypes.h"

// A static class that does platform-agnostic exception handling.  Platform specific logic is done in the
// PlatformExceptionHandler component and then a generic information structure is passed here for processing.
class CStructuredExceptionHandler
{
	public:

		// We include both a standard initialize and one for unit testing that lets you use a custom handler
		static void Initialize( void );
		static void Initialize( const DExceptionHandler &handler );

		static void Shutdown( void );

	private:

		static void On_Structured_Exception_Callback( CStructuredExceptionInfo &shared_exception_info );

		static void Write_Exception_File( const CStructuredExceptionInfo &shared_exception_info );
		static void Archive_Logs( void );

		static std::wstring Get_Log_File_Prefix( void );

};

#endif // STRUCTURED_EXCEPTION_HANDLING_H