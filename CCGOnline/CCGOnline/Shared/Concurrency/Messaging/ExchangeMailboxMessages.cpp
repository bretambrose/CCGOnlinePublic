/**********************************************************************************************************************

	ExchangeInterfaceMessages.cpp
		A component containing definitions for messages that are needed to exchange mailboxes
		between virtual processes

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

#include "stdafx.h"

#include "ExchangeMailboxMessages.h"

#include "Concurrency/MailboxInterfaces.h"

/**********************************************************************************************************************
	CAddMailboxMessage::CAddMailboxMessage -- constructor
	
		write_interface -- a write-only message passing interface to a process
		
**********************************************************************************************************************/
CAddMailboxMessage::CAddMailboxMessage( const shared_ptr< CWriteOnlyMailbox > &mailbox ) :
	Mailbox( mailbox )
{
}

/**********************************************************************************************************************
	CAddMailboxMessage::~CAddMailboxMessage -- destructor
		
**********************************************************************************************************************/
CAddMailboxMessage::~CAddMailboxMessage() 
{
}
