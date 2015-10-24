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

namespace tbb
{
	class task;
}

namespace IP
{
namespace Execution
{

// Defines a virtual process's execution context; currently only contains TBB info
class CProcessExecutionContext
{
	public:

		CProcessExecutionContext( void ) :
			ElapsedTime( 0.0 ),
			IsDirect( false )
		{
		}

		CProcessExecutionContext( tbb::task *task, double elapsed_time ) :
			ElapsedTime( elapsed_time ),
			IsDirect( task == nullptr )
		{
		}

		~CProcessExecutionContext() {}

		double Get_Elapsed_Time( void ) const { return ElapsedTime; }
		bool Is_Direct( void ) const { return IsDirect; }

	private:

		double ElapsedTime;
		bool IsDirect;
};

} // namespace Execution
} // namespace IP