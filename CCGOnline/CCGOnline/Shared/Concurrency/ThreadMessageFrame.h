/**********************************************************************************************************************

	ThreadMessageFrame.h
		A component definining a container of thread messages.  Batching messages into a container leads to more
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

#ifndef THREAD_MESSAGE_FRAME_H
#define THREAD_MESSAGE_FRAME_H

#include "ThreadKey.h"

class IThreadMessage;

// A container of thread messages
class CThreadMessageFrame
{
	public:

		CThreadMessageFrame( const SThreadKey &key ) :
			Key( key ),
			Messages()
		{}

		~CThreadMessageFrame();

		SThreadKey Get_Key( void ) const { return Key; }

		void Add_Message( const shared_ptr< const IThreadMessage > &message );

		std::vector< shared_ptr< const IThreadMessage > >::const_iterator Get_Frame_Begin( void ) const { return Messages.cbegin(); }
		std::vector< shared_ptr< const IThreadMessage > >::const_iterator Get_Frame_End( void ) const { return Messages.cend(); }

	private:

		SThreadKey Key;

		std::vector< shared_ptr< const IThreadMessage > > Messages;

};

#endif // THREAD_MESSAGE_FRAME_H
