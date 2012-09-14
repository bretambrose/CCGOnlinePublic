/**********************************************************************************************************************

	VirtualProcessMessageFrame.h
		A component definining a container of virtual process messages.  Batching messages into a container leads to more
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

#ifndef VIRTUAL_PROCESS_MESSAGE_FRAME_H
#define VIRTUAL_PROCESS_MESSAGE_FRAME_H

class IVirtualProcessMessage;

namespace EVirtualProcessID
{
	enum Enum;
}

// A container of thread messages
class CVirtualProcessMessageFrame
{
	public:

		CVirtualProcessMessageFrame( EVirtualProcessID::Enum process_id ) :
			ProcessID( process_id ),
			Messages()
		{}

		~CVirtualProcessMessageFrame();

		EVirtualProcessID::Enum Get_Process_ID( void ) const { return ProcessID; }

		void Add_Message( const shared_ptr< const IVirtualProcessMessage > &message );

		std::vector< shared_ptr< const IVirtualProcessMessage > >::const_iterator Get_Frame_Begin( void ) const { return Messages.cbegin(); }
		std::vector< shared_ptr< const IVirtualProcessMessage > >::const_iterator Get_Frame_End( void ) const { return Messages.cend(); }

	private:

		EVirtualProcessID::Enum ProcessID;

		std::vector< shared_ptr< const IVirtualProcessMessage > > Messages;

};

#endif // VIRTUAL_PROCESS_MESSAGE_FRAME_H
