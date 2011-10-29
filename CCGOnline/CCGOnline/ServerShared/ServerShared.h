/**********************************************************************************************************************

	[Placeholder for eventual source license]

	ServerShared.h
		Component containing initialize/shutdown for ServerShared library; calls Server library initalize/shutdown

	(c) Copyright 2011, Bret Ambrose.  All rights reserved.

**********************************************************************************************************************/

#ifndef SERVER_SHARED_H
#define SERVER_SHARED_H

namespace NServerShared
{
	void Initialize( void );
	void Shutdown( void );
}

#endif // SERVER_SHARED_H