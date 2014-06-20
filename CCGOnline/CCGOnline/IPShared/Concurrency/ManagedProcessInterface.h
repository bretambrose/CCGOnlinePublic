/**********************************************************************************************************************

	ManagedProcessTaskInterface.h
		A pure virtual interface, underneath IProcess, that adds additional functionality intended for use
		by the concurrency manager.  While technically against object-oriented principles, I wanted to avoid using
		multiple inheritance.  Given that all processes were going to have both interfaces, there didn't seem to be
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

#ifndef MANAGED_PROCESS_INTERFACE_H
#define MANAGED_PROCESS_INTERFACE_H

#include "ProcessInterface.h"

class CWriteOnlyMailbox;
class CReadOnlyMailbox;
class CProcessExecutionContext;

enum ETimeType;

// Pure virtual interface for virtual processes adding functionality needed by the concurrency manager
class IManagedProcess : public IProcess
{
	public:
		
		typedef IProcess BASECLASS;

		IManagedProcess( void ) :
			BASECLASS()
		{}

		virtual ~IManagedProcess() {}

		virtual void Set_Manager_Mailbox( const shared_ptr< CWriteOnlyMailbox > &mailbox ) = 0;
		virtual void Set_Logging_Mailbox( const shared_ptr< CWriteOnlyMailbox > &mailbox ) = 0;
		virtual void Set_My_Mailbox( const shared_ptr< CReadOnlyMailbox > &read_interface ) = 0;

		virtual void Cleanup( void ) = 0;

		virtual ETimeType Get_Time_Type( void ) const = 0;
		virtual bool Is_Root_Thread( void ) const = 0;

		virtual void Run( const CProcessExecutionContext &context ) = 0;

};

#endif // MANAGED_PROCESS_INTERFACE_H