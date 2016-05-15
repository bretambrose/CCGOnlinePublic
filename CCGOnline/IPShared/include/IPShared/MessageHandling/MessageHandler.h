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

#pragma once

#include <IPShared/IPShared.h>

#include <IPCore/Memory/Memory.h>

namespace IP
{

// A simple base class for all message handlers
template< typename MessageSourceType, typename MessageBaseType >
class IMessageHandler
{
	public:

		IMessageHandler( void ) {}
		virtual ~IMessageHandler() = default;

		virtual void Handle_Message( MessageSourceType source, IP::UniquePtr< const MessageBaseType > &message ) const = 0;
};

} // namespace IP