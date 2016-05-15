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

#include <IPCore/Memory/Memory.h>
#include <IPShared/Concurrency/ProcessProperties.h>

namespace IP
{
namespace Concurrency
{

template < typename T > class IConcurrentQueue;
template < typename T > class CTBBConcurrentQueue;
template < typename T > class CLockingConcurrentQueue;

} // namespace Concurrency

namespace Execution
{

class CWriteOnlyMailbox;
class CReadOnlyMailbox;
class CProcessMessageFrame;

enum class EProcessID;

// Controls which concurrent queue implementation we use
//using ProcessToProcessQueueType = CTBBConcurrentQueue< IP::UniquePtr< CProcessMessageFrame > >;
using ProcessToProcessQueueType = IP::Concurrency::CLockingConcurrentQueue< IP::UniquePtr< CProcessMessageFrame > >;

// A class that holds both the read and write interfaces of a thread task
IPSHARED_API class CProcessMailbox
{
	public:

		CProcessMailbox( EProcessID process_id, const SProcessProperties &properties );
		~CProcessMailbox();

		const std::shared_ptr< CWriteOnlyMailbox > &Get_Writable_Mailbox( void ) const { return WriteOnlyMailbox; }
		const std::shared_ptr< CReadOnlyMailbox > &Get_Readable_Mailbox( void ) const { return ReadOnlyMailbox; }

		EProcessID Get_Process_ID( void ) const { return ProcessID; }
		const SProcessProperties &Get_Properties( void ) const { return Properties; }

	private:
		
		EProcessID ProcessID;

		SProcessProperties Properties;

		std::shared_ptr< CWriteOnlyMailbox > WriteOnlyMailbox;
		std::shared_ptr< CReadOnlyMailbox > ReadOnlyMailbox;
};

} // namespace Execution
} // namespace IP