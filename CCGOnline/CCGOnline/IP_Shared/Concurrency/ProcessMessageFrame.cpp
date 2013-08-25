/**********************************************************************************************************************

	ProcessMessageFrame.cpp
		A component definining a container of process messages.  Batching messages into a container leads to more
		efficiency with the concurrency queues in high-traffic situations.

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

#include "ProcessMessageFrame.h"

#include "Messaging/ProcessMessage.h"

/**********************************************************************************************************************
	CProcessMessageFrame::CProcessMessageFrame -- constructor

		process_id -- source of the messages
					
**********************************************************************************************************************/
CProcessMessageFrame::CProcessMessageFrame( EProcessID::Enum process_id ) :
	ProcessID( process_id ),
	Messages()
{
}

/**********************************************************************************************************************
	CProcessMessageFrame::CProcessMessageFrame -- move constructor

		rhs -- move source
					
**********************************************************************************************************************/
CProcessMessageFrame::CProcessMessageFrame( CProcessMessageFrame &&rhs ) :
	ProcessID( rhs.ProcessID ),
	Messages( std::move( rhs.Messages ) )
{
}

/**********************************************************************************************************************
	CProcessMessageFrame::~CProcessMessageFrame -- destructor, defined internally to avoid header dependency on
		IProcessMessage
					
**********************************************************************************************************************/
CProcessMessageFrame::~CProcessMessageFrame()
{
}

/**********************************************************************************************************************
	CProcessMessageFrame::Add_Message -- adds a message to the container

		message -- message to add to the frame
					
**********************************************************************************************************************/
void CProcessMessageFrame::Add_Message( unique_ptr< const IProcessMessage > &message )
{
	Messages.emplace_back( std::move( message ) );
}

/**********************************************************************************************************************
	CProcessMessageFrame::Add_Message -- adds a message to the container

		message -- message to add to the frame
					
**********************************************************************************************************************/
void CProcessMessageFrame::Add_Message( unique_ptr< const IProcessMessage > &&message )
{
	Messages.emplace_back( std::move( message ) );
}




