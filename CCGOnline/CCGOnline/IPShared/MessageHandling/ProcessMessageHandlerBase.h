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

#ifndef PROCESS_MESSAGE_HANDLER_BASE_H
#define PROCESS_MESSAGE_HANDLER_BASE_H

#include "MessageHandler.h"

namespace EProcessID
{
	enum Enum;
}

class IProcessMessage;

// A simple base class for all thread message handlers
class IProcessMessageHandler : public IMessageHandler< EProcessID::Enum, IProcessMessage >
{
	public:

		typedef IMessageHandler< EProcessID::Enum, IProcessMessage > BASECLASS;

		IProcessMessageHandler( void ) :
			BASECLASS()
		{}

		virtual ~IProcessMessageHandler() = default;

};

#endif // PROCESS_MESSAGE_HANDLER_BASE_H

