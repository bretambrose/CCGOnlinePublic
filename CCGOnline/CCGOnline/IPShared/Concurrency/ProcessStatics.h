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
namespace Execution
{

class IProcess;
class CConcurrencyManager;

// Static class managing thread-local handles to the executing process and the concurrency manager. 
class CProcessStatics
{
	public:

		static void Initialize( void );
		static void Shutdown( void );

		static void Set_Current_Process( IProcess *process );
		static void Set_Concurrency_Manager( CConcurrencyManager *manager );

		static IProcess *Get_Current_Process( void );
		static CConcurrencyManager *Get_Concurrency_Manager( void );

	private:

		// Thread Local storage indices
		static uint32_t ProcessHandle;
		static uint32_t ConcurrencyManagerHandle;

		static bool Initialized;
};

} // namespace Execution
} // namespace IP