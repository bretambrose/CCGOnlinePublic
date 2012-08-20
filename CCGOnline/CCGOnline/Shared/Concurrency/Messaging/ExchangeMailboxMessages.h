/**********************************************************************************************************************

	ExchangeMailboxMessages.h
		A component containing definitions for virtual process messages that are needed to exchange process mailboxes
		between processes

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

#ifndef EXCHANGE_INTERFACE_MESSAGES_H
#define EXCHANGE_INTERFACE_MESSAGES_H

#include "VirtualProcessMessage.h"
#include "Concurrency/ThreadKey.h"

class CWriteOnlyMailbox;

// Requests the interface to a thread tasks or set of thread tasks
class CGetMailboxRequest : public IVirtualProcessMessage
{
	public:

		typedef IVirtualProcessMessage BASECLASS;
		
		CGetMailboxRequest( const SThreadKey &source_key, const SThreadKey &target_key ) :
			SourceKey( source_key ),
			TargetKey( target_key )
		{}

		virtual ~CGetMailboxRequest() {}

		const SThreadKey &Get_Source_Key( void ) const { return SourceKey; }
		const SThreadKey &Get_Target_Key( void ) const { return TargetKey; }

	private:

		SThreadKey SourceKey;
		SThreadKey TargetKey;
};

// Requests that the interface of a thread task be pushed to another thread or set of threads
class CPushMailboxRequest : public IVirtualProcessMessage
{
	public:

		typedef IVirtualProcessMessage BASECLASS;
		
		CPushMailboxRequest( const SThreadKey &source_key, const SThreadKey &target_key ) :
			SourceKey( source_key ),
			TargetKey( target_key )
		{}

		virtual ~CPushMailboxRequest() {}

		const SThreadKey &Get_Source_Key( void ) const { return SourceKey; }
		const SThreadKey &Get_Target_Key( void ) const { return TargetKey; }

	private:

		SThreadKey SourceKey;
		SThreadKey TargetKey;
};

// Tells a thread task about an interface to another thread task
class CAddMailboxMessage : public IVirtualProcessMessage
{
	public:
		
		typedef IVirtualProcessMessage BASECLASS;

		CAddMailboxMessage( const SThreadKey &key, const shared_ptr< CWriteOnlyMailbox > &mailbox );

		virtual ~CAddMailboxMessage();

		shared_ptr< CWriteOnlyMailbox > Get_Mailbox( void ) const { return Mailbox; }
		const SThreadKey &Get_Key( void ) const { return Key; }

	private:

		SThreadKey Key;

		shared_ptr< CWriteOnlyMailbox > Mailbox;
};


#endif // EXCHANGE_MAILBOX_MESSAGES_H