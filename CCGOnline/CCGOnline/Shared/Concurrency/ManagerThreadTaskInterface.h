/**********************************************************************************************************************

	ManagerThreadTaskInterface.h
		A pure virtual interface, underneath IThreadTask, that adds additional functionality intended for use
		by the concurrency manager.  While technically against object-oriented principles, I wanted to avoid using
		multiple inheritance.  Given that all threads were going to have both interfaces, there didn't seem to be
		a point in splitting them, and the necessary casts seemed ugly in the multiple inheritance case.

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

#ifndef MANAGER_THREAD_TASK_INTERFACE_H
#define MANAGER_THREAD_TASK_INTERFACE_H

#include "ThreadTaskInterface.h"

class CWriteOnlyThreadInterface;
class CReadOnlyThreadInterface;
class CThreadTaskExecutionContext;

enum ETimeType;

// Pure virtual interface for thread tasks adding functionality needed by the concurrency manager
class IManagerThreadTask : public IThreadTask
{
	public:
		
		typedef IThreadTask BASECLASS;

		IManagerThreadTask( void ) :
			BASECLASS()
		{}

		virtual ~IManagerThreadTask() {}

		virtual void Set_Key( const SThreadKey &key ) = 0;

		virtual void Set_Manager_Interface( const shared_ptr< CWriteOnlyThreadInterface > &write_interface ) = 0;
		virtual void Set_Read_Interface( const shared_ptr< CReadOnlyThreadInterface > &read_interface ) = 0;

		virtual void Cleanup( void ) = 0;

		virtual ETimeType Get_Time_Type( void ) const = 0;
		virtual bool Is_Root_Thread( void ) const = 0;

		virtual void Service( double elapsed_seconds, const CThreadTaskExecutionContext &context ) = 0;

		virtual double Get_Elapsed_Seconds( void ) const = 0;

};

#endif // MANAGER_THREAD_TASK_INTERFACE_H