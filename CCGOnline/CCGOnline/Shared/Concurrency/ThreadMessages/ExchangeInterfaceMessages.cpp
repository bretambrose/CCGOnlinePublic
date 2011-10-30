/**********************************************************************************************************************

	ExchangeInterfaceMessages.cpp
		A component containing definitions for thread messages that are needed to exchange thread interfaces
		between threads

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
