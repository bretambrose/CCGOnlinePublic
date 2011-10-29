/**********************************************************************************************************************

	[Placeholder for eventual source license]

	ExchangeInterfaceMessages.cpp
		A component containing definitions for thread messages that are needed to exchange thread interfaces
		between threads

	(c) Copyright 2011, Bret Ambrose.  All rights reserved.

**********************************************************************************************************************/

#include "stdafx.h"

#include "ExchangeInterfaceMessages.h"

#include "Concurrency/ThreadSubject.h"
#include "Concurrency/ThreadInterfaces.h"
#include "Concurrency/ThreadConstants.h"

// Defined in a cpp file in order to keep dependencies out of the header file

/**********************************************************************************************************************
	CAddInterfaceMessage::CAddInterfaceMessage -- constructor
	
		key -- thread key the supplied interface corresponds to
		write_interface -- a write-only message passing interface to a thread task
		
**********************************************************************************************************************/
CAddInterfaceMessage::CAddInterfaceMessage( const SThreadKey &key, const shared_ptr< CWriteOnlyThreadInterface > &write_interface ) :
	Key( key ),
	WriteInterface( write_interface )
{
}

/**********************************************************************************************************************
	CAddInterfaceMessage::~CAddInterfaceMessage -- destructor
		
**********************************************************************************************************************/
CAddInterfaceMessage::~CAddInterfaceMessage() 
{
}
