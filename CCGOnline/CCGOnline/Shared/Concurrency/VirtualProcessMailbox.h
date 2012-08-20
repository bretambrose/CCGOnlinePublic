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

#include "ThreadKey.h"

class CWriteOnlyMailbox;
class CReadOnlyMailbox;
class CVirtualProcessMessageFrame;
template < typename T > class IConcurrentQueue;

// A class that holds both the read and write interfaces of a thread task
class CVirtualProcessMailbox
{
	public:

		CVirtualProcessMailbox( const SThreadKey &key );
		~CVirtualProcessMailbox();

		const shared_ptr< CWriteOnlyMailbox > &Get_Writable_Mailbox( void ) const { return WriteOnlyMailbox; }
		const shared_ptr< CReadOnlyMailbox > &Get_Readable_Mailbox( void ) const { return ReadOnlyMailbox; }

		const SThreadKey &Get_Key( void ) const { return Key; }

	private:
		
		SThreadKey Key;

		shared_ptr< IConcurrentQueue< shared_ptr< CVirtualProcessMessageFrame > > > Queue;

		shared_ptr< CWriteOnlyMailbox > WriteOnlyMailbox;
		shared_ptr< CReadOnlyMailbox > ReadOnlyMailbox;
};

#endif // VIRTUAL_PROCESS_MAILBOX_H