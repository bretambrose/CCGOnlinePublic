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

#pragma once

#include <IPCore/IPCore.h>

#include <IPCore/Memory/Stl/String.h>

#include <functional>

namespace IP
{
namespace Debug
{
namespace Assert
{

	using DLogFunctionType = std::function< void(IP::String &) > ;

	IPCORE_API void Initialize( const DLogFunctionType &log_function );
	IPCORE_API void Shutdown( void );

	// all forms of condition checking go here on failure
	IPCORE_API bool Assert_Handler( const char *expression_string, const char *file_name, uint32_t line_number, bool force_crash );

} // namespace Assert
} // namespace Debug
} // namespace IP

// Macros
#ifdef _DEBUG

#define FAIL_IF( x ) if ( ( x ) && IP::Debug::Assert::Assert_Handler( #x, __FILE__, __LINE__, false ) )
#define DEBUG_ASSERT( x ) ( ( x ) || IP::Debug::Assert::Assert_Handler( #x, __FILE__, __LINE__, false ) )

#else

#define FAIL_IF( x ) if ( x )
#define DEBUG_ASSERT( x )

#endif // _DEBUG

#define FATAL_ASSERT( x ) ( ( x ) || IP::Debug::Assert::Assert_Handler( #x, __FILE__, __LINE__, true ) )

