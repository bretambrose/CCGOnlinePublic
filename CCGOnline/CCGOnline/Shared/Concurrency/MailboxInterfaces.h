/**********************************************************************************************************************

	MailboxInterfaces.h
		A component definining the read-only and write-only interfaces that a mailbox has

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

#ifndef MAILBOX_INTERFACES_H
#define MAILBOX_INTERFACES_H

#include "ProcessProperties.h"

class CProcessMessageFrame;
template < typename T > class IConcurrentQueue;

namespace EProcessID
{
	enum Enum;
}

// The write-only mailbox of a virtual process.  Other processes talk to a process by adding messages to this
class CWriteOnlyMailbox
{
	public:

		CWriteOnlyMailbox( EProcessID::Enum process_id, const SProcessProperties &properties, const shared_ptr< IConcurrentQueue< unique_ptr< CProcessMessageFrame > > > &write_queue );
		~CWriteOnlyMailbox();

		EProcessID::Enum Get_Process_ID( void ) const { return ProcessID; }
		const SProcessProperties &Get_Properties( void ) const { return Properties; }

		void Add_Frame( unique_ptr< CProcessMessageFrame > &frame );

	private:

		EProcessID::Enum ProcessID;

		SProcessProperties Properties;

		shared_ptr< IConcurrentQueue< unique_ptr< CProcessMessageFrame > > > WriteQueue;

};

// The read-only mailbox to a virtual process.  A process handles messages by reading from this
class CReadOnlyMailbox
{
	public:

		CReadOnlyMailbox( const shared_ptr< IConcurrentQueue< unique_ptr< CProcessMessageFrame > > > &read_queue );
		~CReadOnlyMailbox();

		void Remove_Frames( std::vector< unique_ptr< CProcessMessageFrame > > &frames );

	private:

		shared_ptr< IConcurrentQueue< unique_ptr< CProcessMessageFrame > > > ReadQueue;

};

#endif // MAILBOX_INTERFACES_H