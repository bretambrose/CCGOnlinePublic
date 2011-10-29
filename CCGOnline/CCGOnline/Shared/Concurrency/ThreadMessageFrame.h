/**********************************************************************************************************************

	[Placeholder for eventual source license]

	ThreadMessageFrame.h
		A component definining a container of thread messages.  Batching messages into a container leads to more
		efficiency with the concurrency queues in high-traffic situations.

	(c) Copyright 2011, Bret Ambrose.  All rights reserved.

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
