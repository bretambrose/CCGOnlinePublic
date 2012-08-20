/**********************************************************************************************************************

	VirtualProcessMessageHandlerBase.h
		A component defining a bare base class for objects that handle virtual process messages.  In practice these objects
		are simple trampolines that forward handling to a delegate while retaining type safety and
		generic behavior.

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

#ifndef VIRTUAL_PROCESS_MESSAGE_HANDLER_BASE_H
#define VIRTUAL_PROCESS_MESSAGE_HANDLER_BASE_H

#include "MessageHandler.h"

struct SThreadKey;
class IVirtualProcessMessage;

// A simple base class for all thread message handlers
class IVirtualProcessMessageHandler : public IMessageHandler< SThreadKey, IVirtualProcessMessage >
{
	public:

		typedef IMessageHandler< SThreadKey, IVirtualProcessMessage > BASECLASS;

		IVirtualProcessMessageHandler( void ) :
			BASECLASS()
		{}

		virtual ~IVirtualProcessMessageHandler() {}

};

#endif // VIRTUAL_PROCESS_MESSAGE_HANDLER_BASE_H

