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

#include <IPShared/Concurrency/Messaging/ProcessMessage.h>
#include <IPShared/Concurrency/ProcessProperties.h>

#include <memory>

namespace IP
{
namespace Execution
{

class CWriteOnlyMailbox;
enum class EProcessID;

namespace Messaging
{

// Requests the interface to a thread tasks or set of thread tasks
IPSHARED_API class CGetMailboxByPropertiesRequest : public IProcessMessage
{
	public:

		using BASECLASS = IProcessMessage;
		
		CGetMailboxByPropertiesRequest( const IP::Execution::SProcessProperties &target_properties ) :
			TargetProperties( target_properties )
		{}

		virtual ~CGetMailboxByPropertiesRequest() = default;

		const IP::Execution::SProcessProperties &Get_Target_Properties( void ) const { return TargetProperties; }

	private:

		IP::Execution::SProcessProperties TargetProperties;
};

IPSHARED_API class CGetMailboxByIDRequest : public IProcessMessage
{
	public:

		using BASECLASS = IProcessMessage;
		
		CGetMailboxByIDRequest( IP::Execution::EProcessID target_process_id ) :
			TargetProcessID( target_process_id )
		{}

		virtual ~CGetMailboxByIDRequest() = default;

		IP::Execution::EProcessID Get_Target_Process_ID( void ) const { return TargetProcessID; }

	private:

		IP::Execution::EProcessID TargetProcessID;
};

// Tells a thread task about an interface to another thread task
IPSHARED_API class CAddMailboxMessage : public IProcessMessage
{
	public:
		
		using BASECLASS = IProcessMessage;

		CAddMailboxMessage( const std::shared_ptr< IP::Execution::CWriteOnlyMailbox > &mailbox );

		virtual ~CAddMailboxMessage();

		const std::shared_ptr< IP::Execution::CWriteOnlyMailbox > &Get_Mailbox( void ) const { return Mailbox; }

	private:

		std::shared_ptr< IP::Execution::CWriteOnlyMailbox > Mailbox;
};


} // namespace Messaging
} // namespace Execution
} // namespace IP
