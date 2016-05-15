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

#include <IPShared/Concurrency/ProcessMailbox.h>

#include <IPShared/Concurrency/MailboxInterfaces.h>
#include <IPShared/Concurrency/Containers/TBBConcurrentQueue.h>
#include <IPShared/Concurrency/Containers/LockingConcurrentQueue.h>
#include <IPShared/Concurrency/ProcessMessageFrame.h>

namespace IP
{
namespace Execution
{

CProcessMailbox::CProcessMailbox( EProcessID process_id, const SProcessProperties &properties ) :
	ProcessID( process_id ),
	Properties( properties ),
	WriteOnlyMailbox( nullptr ),
	ReadOnlyMailbox( nullptr )
{
	std::shared_ptr< IP::Concurrency::IConcurrentQueue< IP::UniquePtr< CProcessMessageFrame > > > queue = std::static_pointer_cast< IP::Concurrency::IConcurrentQueue< IP::UniquePtr< CProcessMessageFrame > > >( IP::Make_Shared< ProcessToProcessQueueType >( MEMORY_TAG ) );

	WriteOnlyMailbox = IP::Make_Shared< CWriteOnlyMailbox >( MEMORY_TAG, process_id, properties, queue );
	ReadOnlyMailbox = IP::Make_Shared< CReadOnlyMailbox >( MEMORY_TAG, queue );
}


CProcessMailbox::~CProcessMailbox()
{
}

} // namespace Execution
} // namespace IP