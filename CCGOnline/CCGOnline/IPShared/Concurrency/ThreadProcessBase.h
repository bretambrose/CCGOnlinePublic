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

#ifndef THREAD_PROCESS_BASE_H
#define THREAD_PROCESS_BASE_H

#include "ProcessBase.h"

// The shared logic level of all thread-based virtual processes; not instantiable
class CThreadProcessBase : public CProcessBase
{
	public:
		
		typedef CProcessBase BASECLASS;

		// Construction/destruction
		CThreadProcessBase( const SProcessProperties &properties );
		virtual ~CThreadProcessBase();

		// IThreadTask interface
		virtual EProcessExecutionMode::Enum Get_Execution_Mode( void ) const override;

		virtual void Run( const CProcessExecutionContext &context ) override;

	protected:

		virtual double Get_Current_Process_Time( void ) const override;

		virtual uint32_t Get_Sleep_Interval_In_Milliseconds( void ) const = 0;

	private:

		void Thread_Function( void *thread_data );

		friend class CProcessBaseTester;
		friend class CProcessBaseExaminer;

		// Private Data

		// Misc
		bool HasBeenRun;	// Do not move this to the base class
};

#endif // THREAD_PROCESS_BASE_H
