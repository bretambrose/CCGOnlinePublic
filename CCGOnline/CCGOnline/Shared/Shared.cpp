/**********************************************************************************************************************

	[Placeholder for eventual source license]

	Shared.cpp
		Component containing initialization and shutdown entry points for the Shared library

	(c) Copyright 2011, Bret Ambrose.  All rights reserved.

**********************************************************************************************************************/

#include "stdafx.h"

#include "Shared.h"

#include "Logging/LogInterface.h"
#include "StructuredExceptionHandler.h"
#include "Concurrency/ThreadStatics.h"
#include "PlatformTime.h"
#include "PlatformProcess.h"

/**********************************************************************************************************************
	NShared::Initialize -- Initializes all the static process-wide systems in the Shared library

**********************************************************************************************************************/
void NShared::Initialize( void )
{
	CAssertSystem::Initialize( DLogFunctionType( CLogInterface::Log ) );
	CStructuredExceptionHandler::Initialize();
	CLogInterface::Initialize_Static( NPlatform::Get_Service_Name(), LL_LOW );
	CThreadStatics::Initialize();
	CPlatformTime::Initialize();	// does not have a corresponding shutdown function
}

/**********************************************************************************************************************
	NShared::Shutdown -- Shuts down and cleans up all the static process-wide systems in the Shared library

**********************************************************************************************************************/
void NShared::Shutdown( void )
{
	CThreadStatics::Shutdown();
	CLogInterface::Shutdown_Static();
	CStructuredExceptionHandler::Shutdown();
	CAssertSystem::Shutdown();
}