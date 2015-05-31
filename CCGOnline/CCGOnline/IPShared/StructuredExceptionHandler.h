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

#ifndef STRUCTURED_EXCEPTION_HANDLER_H
#define STRUCTURED_EXCEPTION_HANDLER_H

#include "IPPlatform/StructuredExceptionTypes.h"

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