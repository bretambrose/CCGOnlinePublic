/**********************************************************************************************************************

	VirtualProcessMailbox.h
		A component definining the class that holds both the read and write mailboxes of a virtual process.  Only the
		manager has access to this.

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

#ifndef VIRTUAL_PROCESS_MAILBOX_H
#define VIRTUAL_PROCESS_MAILBOX_H

#include "VirtualProcessProperties.h"

class CWriteOnlyMailbox;
class CReadOnlyMailbox;
class CVirtualProcessMessageFrame;
template < typename T > class IConcurrentQueue;

namespace EVirtualProcessID
{
	enum Enum;
}

// A class that holds both the read and write interfaces of a thread task
class CVirtualProcessMailbox
{
	public:

		CVirtualProcessMailbox( EVirtualProcessID::Enum process_id, const SProcessProperties &properties );
		~CVirtualProcessMailbox();

		const shared_ptr< CWriteOnlyMailbox > &Get_Writable_Mailbox( void ) const { return WriteOnlyMailbox; }
		const shared_ptr< CReadOnlyMailbox > &Get_Readable_Mailbox( void ) const { return ReadOnlyMailbox; }

		EVirtualProcessID::Enum Get_Process_ID( void ) const { return ProcessID; }
		const SProcessProperties &Get_Properties( void ) const { return Properties; }

	private:
		
		EVirtualProcessID::Enum ProcessID;

		SProcessProperties Properties;

		shared_ptr< CWriteOnlyMailbox > WriteOnlyMailbox;
		shared_ptr< CReadOnlyMailbox > ReadOnlyMailbox;
};

#endif // VIRTUAL_PROCESS_MAILBOX_H