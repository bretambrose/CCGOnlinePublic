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
#include <IPCore/Memory/Stl/Vector.h>
#include <IPShared/Concurrency/ProcessProperties.h>

namespace IP
{
namespace Concurrency
{

template < typename T > class IConcurrentQueue;

} // namespace Concurrency

namespace Execution
{

class CProcessMessageFrame;

enum class EProcessID;

// The write-only mailbox of a virtual process.  Other processes talk to a process by adding messages to this
IPSHARED_API class CWriteOnlyMailbox
{
	public:

		CWriteOnlyMailbox( EProcessID process_id, const SProcessProperties &properties, const std::shared_ptr< IP::Concurrency::IConcurrentQueue< IP::UniquePtr< CProcessMessageFrame > > > &write_queue );
		~CWriteOnlyMailbox();

		CWriteOnlyMailbox( CWriteOnlyMailbox &&rhs ) = delete;
		CWriteOnlyMailbox & operator =( CWriteOnlyMailbox &&rhs ) = delete;
		CWriteOnlyMailbox( const CWriteOnlyMailbox &rhs ) = delete;
		CWriteOnlyMailbox & operator =( const CWriteOnlyMailbox &rhs ) = delete;

		EProcessID Get_Process_ID( void ) const { return ProcessID; }
		const SProcessProperties &Get_Properties( void ) const { return Properties; }

		void Add_Frame( IP::UniquePtr< CProcessMessageFrame > &frame );

	private:

		EProcessID ProcessID;

		SProcessProperties Properties;

		std::shared_ptr< IP::Concurrency::IConcurrentQueue< IP::UniquePtr< CProcessMessageFrame > > > WriteQueue;

};

// The read-only mailbox to a virtual process.  A process handles messages by reading from this
IPSHARED_API class CReadOnlyMailbox
{
	public:

		CReadOnlyMailbox( const std::shared_ptr< IP::Concurrency::IConcurrentQueue< IP::UniquePtr< CProcessMessageFrame > > > &read_queue );
		~CReadOnlyMailbox();

		CReadOnlyMailbox( CReadOnlyMailbox &&rhs ) = delete;
		CReadOnlyMailbox & operator =( CReadOnlyMailbox &&rhs ) = delete;
		CReadOnlyMailbox( const CReadOnlyMailbox &rhs ) = delete;
		CReadOnlyMailbox & operator =( const CReadOnlyMailbox &rhs ) = delete;

		void Remove_Frames( IP::Vector< IP::UniquePtr< CProcessMessageFrame > > &frames );

	private:

		std::shared_ptr< IP::Concurrency::IConcurrentQueue< IP::UniquePtr< CProcessMessageFrame > > > ReadQueue;

};

} // namespace Execution
} // namespace IP
