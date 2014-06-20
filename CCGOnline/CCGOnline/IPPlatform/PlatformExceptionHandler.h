/**********************************************************************************************************************

	PlatformExceptionHandler.h
		A component that wraps OS-specific exception handling logic, including generating a stack trace

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