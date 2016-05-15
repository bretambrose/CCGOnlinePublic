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

#include <IPShared/Concurrency/Messaging/ProcessManagementMessages.h>

#include "IPShared/Concurrency/ProcessInterface.h"

namespace IP
{
namespace Execution
{
namespace Messaging
{

CAddNewProcessMessage::CAddNewProcessMessage( const std::shared_ptr< IP::Execution::IProcess > &process, bool return_mailbox, bool forward_creator_mailbox ) :
	Process( process ),
	ReturnMailbox( return_mailbox ),
	ForwardCreatorMailbox( forward_creator_mailbox )
{
}


CAddNewProcessMessage::~CAddNewProcessMessage()
{
}

} // namespace Messaging
} // namespace Execution
} // namespace IP
