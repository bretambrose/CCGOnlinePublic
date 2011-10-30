/**********************************************************************************************************************

	MessageHandler.h
		A component defining a top-level base class for objects that handle a set of related message classes.  In 
		practice these objects are simple trampolines that forward handling to a delegate while retaining type safety 
		and generic behavior.

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

#ifndef MESSAGE_HANDLER_H
#define MESSAGE_HANDLER_H

// A simple base class for all message handlers
template< typename MessageSourceType, typename MessageBaseType >
class IMessageHandler
{
	public:

		IMessageHandler( void ) {}
		virtual ~IMessageHandler() {}

		virtual void Handle_Message( const MessageSourceType &source, const shared_ptr< const MessageBaseType > &message ) const = 0;
};

#endif // MESSAGE_HANDLER_H
