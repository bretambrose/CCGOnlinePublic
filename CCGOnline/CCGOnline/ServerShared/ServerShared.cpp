/**********************************************************************************************************************

	[Placeholder for eventual source license]

	ServerShared.cpp
		Component containing  initialize/shutdown for ServerShared library

	(c) Copyright 2011, Bret Ambrose.  All rights reserved.

**********************************************************************************************************************/

#include "stdafx.h"

#include "ServerShared.h"

#include "Shared.h"

/**********************************************************************************************************************
	NServerShared::Initialize_Server_Shared -- Initializes the ServerShared library and all its ancestors

**********************************************************************************************************************/
void NServerShared::Initialize( void )
{
	NShared::Initialize();
}

/**********************************************************************************************************************
	NServerShared::Shutdown_Server_Shared -- Shuts down the ServerShared library and all its ancestors

**********************************************************************************************************************/
void  NServerShared::Shutdown( void )
{
	NShared::Shutdown();
}