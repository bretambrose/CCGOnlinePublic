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

namespace IP
{
namespace Debug
{

using DLogFunctionType = FastDelegate1< std::wstring &, void > ;

// A static class to handle all assertion failures.  Multi-threaded safe via a mutex.
class CAssertSystem
{
	public:

		static void Initialize( const DLogFunctionType &log_function );
		static void Shutdown( void );

		// all forms of condition checking go here on failure
		static bool Assert_Handler( const char *expression_string, const char *file_name, uint32_t line_number, bool force_crash );

	private:

		static std::mutex AssertLock;

		static DLogFunctionType LogFunction;

		static std::atomic< bool > Initialized;
};

} // namespace Debug
} // namespace IP

// Macros
#ifdef _DEBUG

#define FAIL_IF( x ) if ( ( x ) && IP::Debug::CAssertSystem::Assert_Handler( #x, __FILE__, __LINE__, false ) )
#define DEBUG_ASSERT( x ) ( ( x ) || IP::Debug::CAssertSystem::Assert_Handler( #x, __FILE__, __LINE__, false ) )

#else

#define FAIL_IF( x ) if ( x )
#define DEBUG_ASSERT( x )

#endif // _DEBUG

#define FATAL_ASSERT( x ) ( ( x ) || IP::Debug::CAssertSystem::Assert_Handler( #x, __FILE__, __LINE__, true ) )

