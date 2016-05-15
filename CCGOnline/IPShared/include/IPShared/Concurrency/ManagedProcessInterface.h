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

#include <IPShared/IPShared.h>

#include <IPShared/Concurrency/ProcessInterface.h>

#include <memory>

namespace IP
{
namespace Execution
{

class CWriteOnlyMailbox;
class CReadOnlyMailbox;
class CProcessExecutionContext;

// Pure virtual interface for virtual processes adding functionality needed by the concurrency manager
IPSHARED_API class IManagedProcess : public IProcess
{
	public:
		
		using BASECLASS = IProcess;

		IManagedProcess( void ) :
			BASECLASS()
		{}

		virtual ~IManagedProcess() = default;

		virtual void Set_Manager_Mailbox( const std::shared_ptr< CWriteOnlyMailbox > &mailbox ) = 0;
		virtual void Set_Logging_Mailbox( const std::shared_ptr< CWriteOnlyMailbox > &mailbox ) = 0;
		virtual void Set_My_Mailbox( const std::shared_ptr< CReadOnlyMailbox > &read_interface ) = 0;

		virtual void Cleanup( void ) = 0;

		virtual bool Is_Root_Thread( void ) const = 0;

		virtual void Run( const CProcessExecutionContext &context ) = 0;
		virtual void Finalize( void ) = 0;

};

} // namespace Execution
} // namespace IP