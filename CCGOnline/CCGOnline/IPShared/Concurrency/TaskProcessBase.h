/**********************************************************************************************************************

	TaskProcessBase.h
		A component containing the logic shared by all task-based processes.

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

#ifndef TASK_PROCESS_BASE_H
#define TASK_PROCESS_BASE_H

#include "ProcessBase.h"

// The shared logic level of all task-based virtual processes; not instantiable
class CTaskProcessBase : public CProcessBase
{
	public:
		
		typedef CProcessBase BASECLASS;

		// Construction/destruction
		CTaskProcessBase( const SProcessProperties &properties );
		virtual ~CTaskProcessBase();

		// IThreadTask interface
		virtual EProcessExecutionMode::Enum Get_Execution_Mode( void ) const override;

		virtual void Run( const CProcessExecutionContext &context ) override;

	protected:

		double Get_Current_Process_Time( void ) const;

		// Scheduling interface, intended for derived classes
		virtual double Get_Reschedule_Time( void ) const;
		virtual double Get_Reschedule_Interval( void ) const;

		// Base schedule interface
		virtual void Service_Reschedule( void ) override;

	private:

		friend class CTaskProcessBaseTester;
		friend class CTaskProcessBaseExaminer;

		// Private Data
		// Timing
		double FirstServiceTimeSeconds;
		double CurrentTimeSeconds;

		bool HasBeenRun; // Do not move this to the base class

};

#endif // TASK_PROCESS_BASE_H
