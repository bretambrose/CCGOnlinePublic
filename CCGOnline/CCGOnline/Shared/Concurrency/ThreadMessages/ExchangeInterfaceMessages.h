/**********************************************************************************************************************

	[Placeholder for eventual source license]

	ExchangeInterfaceMessages.h
		A component containing definitions for thread messages that are needed to exchange thread interfaces
		between threads

	(c) Copyright 2011, Bret Ambrose.  All rights reserved.

**********************************************************************************************************************/

#ifndef EXCHANGE_INTERFACE_MESSAGES_H
#define EXCHANGE_INTERFACE_MESSAGES_H

#include "ThreadMessage.h"
#include "Concurrency/ThreadKey.h"

class CWriteOnlyThreadInterface;

// Requests the interface to a thread tasks or set of thread tasks
class CGetInterfaceRequest : public IThreadMessage
{
	public:

		typedef IThreadMessage BASECLASS;
		
		CGetInterfaceRequest( const SThreadKey &source_key, const SThreadKey &target_key ) :
			SourceKey( source_key ),
			TargetKey( target_key )
		{}

		virtual ~CGetInterfaceRequest() {}

		const SThreadKey &Get_Source_Key( void ) const { return SourceKey; }
		const SThreadKey &Get_Target_Key( void ) const { return TargetKey; }

	private:

		SThreadKey SourceKey;
		SThreadKey TargetKey;
};

// Requests that the interface of a thread task be pushed to another thread or set of threads
class CPushInterfaceRequest : public IThreadMessage
{
	public:

		typedef IThreadMessage BASECLASS;
		
		CPushInterfaceRequest( const SThreadKey &source_key, const SThreadKey &target_key ) :
			SourceKey( source_key ),
			TargetKey( target_key )
		{}

		virtual ~CPushInterfaceRequest() {}

		const SThreadKey &Get_Source_Key( void ) const { return SourceKey; }
		const SThreadKey &Get_Target_Key( void ) const { return TargetKey; }

	private:

		SThreadKey SourceKey;
		SThreadKey TargetKey;
};

// Tells a thread task about an interface to another thread task
class CAddInterfaceMessage : public IThreadMessage
{
	public:
		
		typedef IThreadMessage BASECLASS;

		CAddInterfaceMessage( const SThreadKey &key, const shared_ptr< CWriteOnlyThreadInterface > &write_interface );

		virtual ~CAddInterfaceMessage();

		shared_ptr< CWriteOnlyThreadInterface > Get_Interface( void ) const { return WriteInterface; }
		const SThreadKey &Get_Key( void ) const { return Key; }

	private:

		SThreadKey Key;

		shared_ptr< CWriteOnlyThreadInterface > WriteInterface;
};


#endif // EXCHANGE_INTERFACE_MESSAGES_H