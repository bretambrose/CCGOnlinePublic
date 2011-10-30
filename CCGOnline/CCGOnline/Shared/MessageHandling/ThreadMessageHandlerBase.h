/**********************************************************************************************************************

	ThreadMessageHandlerBase.h
		A component defining a bare base class for objects that handle thread messages.  In practice these objects
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

#ifndef THREAD_MESSAGE_HANDLER_BASE_H
#define THREAD_MESSAGE_HANDLER_BASE_H

#include "MessageHandler.h"

struct SThreadKey;
class IThreadMessage;

// A simple base class for all thread message handlers
class IThreadMessageHandler : public IMessageHandler< SThreadKey, IThreadMessage >
{
	public:

		typedef IMessageHandler< SThreadKey, IThreadMessage > BASECLASS;

		IThreadMessageHandler( void ) :
			BASECLASS()
		{}

		virtual ~IThreadMessageHandler() {}

};

#endif // THREAD_MESSAGE_HANDLER_BASE_H
