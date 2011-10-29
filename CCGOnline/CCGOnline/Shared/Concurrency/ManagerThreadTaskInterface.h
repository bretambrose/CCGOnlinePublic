/**********************************************************************************************************************

	[Placeholder for eventual source license]

	ManagerThreadTaskInterface.h
		A pure virtual interface, underneath IThreadTask, that adds additional functionality intended for use
		by the concurrency manager.  While technically against object-oriented principles, I wanted to avoid using
		multiple inheritance.  Given that all threads were going to have both interfaces, there didn't seem to be
		a point in splitting them, and the necessary casts seemed ugly in the multiple inheritance case.

	(c) Copyright 2011, Bret Ambrose.  All rights reserved.

**********************************************************************************************************************/

#ifndef MANAGER_THREAD_TASK_INTERFACE_H
#define MANAGER_THREAD_TASK_INTERFACE_H

#include "ThreadTaskInterface.h"

class CWriteOnlyThreadInterface;
class CReadOnlyThreadInterface;
class CThreadTaskExecutionContext;

enum ETimeType;

// Pure virtual interface for thread tasks adding functionality needed by the concurrency manager
class IManagerThreadTask : public IThreadTask
{
	public:
		
		typedef IThreadTask BASECLASS;

		IManagerThreadTask( void ) :
			BASECLASS()
		{}

		virtual ~IManagerThreadTask() {}

		virtual void Set_Key( const SThreadKey &key ) = 0;

		virtual void Set_Manager_Interface( const shared_ptr< CWriteOnlyThreadInterface > &write_interface ) = 0;
		virtual void Set_Read_Interface( const shared_ptr< CReadOnlyThreadInterface > &read_interface ) = 0;

		virtual void Cleanup( void ) = 0;

		virtual ETimeType Get_Time_Type( void ) const = 0;
		virtual bool Is_Root_Thread( void ) const = 0;

		virtual void Service( double elapsed_seconds, const CThreadTaskExecutionContext &context ) = 0;

		virtual double Get_Elapsed_Seconds( void ) const = 0;

};

#endif // MANAGER_THREAD_TASK_INTERFACE_H