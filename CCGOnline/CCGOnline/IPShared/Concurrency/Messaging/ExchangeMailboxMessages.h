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

#ifndef EXCHANGE_INTERFACE_MESSAGES_H
#define EXCHANGE_INTERFACE_MESSAGES_H

#include "ProcessMessage.h"
#include "IPShared/Concurrency/ProcessProperties.h"

class CWriteOnlyMailbox;

namespace EProcessID
{
	enum Enum;
}

// Requests the interface to a thread tasks or set of thread tasks
class CGetMailboxByPropertiesRequest : public IProcessMessage
{
	public:

		typedef IProcessMessage BASECLASS;
		
		CGetMailboxByPropertiesRequest( const SProcessProperties &target_properties ) :
			TargetProperties( target_properties )
		{}

		virtual ~CGetMailboxByPropertiesRequest() = default;

		const SProcessProperties &Get_Target_Properties( void ) const { return TargetProperties; }

	private:

		SProcessProperties TargetProperties;
};

class CGetMailboxByIDRequest : public IProcessMessage
{
	public:

		typedef IProcessMessage BASECLASS;
		
		CGetMailboxByIDRequest( EProcessID::Enum target_process_id ) :
			TargetProcessID( target_process_id )
		{}

		virtual ~CGetMailboxByIDRequest() = default;

		EProcessID::Enum Get_Target_Process_ID( void ) const { return TargetProcessID; }

	private:

		EProcessID::Enum TargetProcessID;
};

// Tells a thread task about an interface to another thread task
class CAddMailboxMessage : public IProcessMessage
{
	public:
		
		typedef IProcessMessage BASECLASS;

		CAddMailboxMessage( const shared_ptr< CWriteOnlyMailbox > &mailbox );

		virtual ~CAddMailboxMessage();

		const shared_ptr< CWriteOnlyMailbox > &Get_Mailbox( void ) const { return Mailbox; }

	private:

		shared_ptr< CWriteOnlyMailbox > Mailbox;
};


#endif // EXCHANGE_MAILBOX_MESSAGES_H