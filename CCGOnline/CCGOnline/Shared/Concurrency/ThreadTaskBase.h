/**********************************************************************************************************************

	[Placeholder for eventual source license]

	ThreadTaskBase.h
		A component containing the logic shared by all thread tasks.

	(c) Copyright 2011, Bret Ambrose.  All rights reserved.

**********************************************************************************************************************/

#ifndef THREAD_TASK_BASE_H
#define THREAD_TASK_BASE_H

#include "ManagerThreadTaskInterface.h"

#include "ThreadKey.h"
#include "TypeInfoUtils.h"

class CAddInterfaceMessage;
class CShutdownInterfaceMessage;
class CShutdownThreadRequest;
class IThreadMessageHandler;
class CThreadMessageFrame;

struct STickTime;

enum EThreadState;

// The shared logic level of all thread tasks; not instantiable
class CThreadTaskBase : public IManagerThreadTask
{
	public:
		
		typedef IManagerThreadTask BASECLASS;

		// Construction/destruction
		CThreadTaskBase( const SThreadKey &key );
		virtual ~CThreadTaskBase();

		// IThreadTask interface
		virtual void Initialize( void );

		virtual const SThreadKey &Get_Key( void ) const { return Key; }

		virtual void Send_Thread_Message( const SThreadKey &dest_key, const shared_ptr< const IThreadMessage > &message );
		virtual void Log( const std::wstring &message );

		virtual CTaskScheduler *Get_Task_Scheduler( void ) const { return TaskScheduler.get(); }

		virtual void Flush_Partitioned_Messages( void );

		// IManagerThreadTask interface
		virtual void Set_Key( const SThreadKey &key ) { Key = key; }

		virtual void Set_Manager_Interface( const shared_ptr< CWriteOnlyThreadInterface > &write_interface );
		virtual void Set_Read_Interface( const shared_ptr< CReadOnlyThreadInterface > &read_interface );

		virtual void Cleanup( void );

		virtual void Service( double elapsed_seconds, const CThreadTaskExecutionContext &context );

		virtual double Get_Elapsed_Seconds( void ) const;

	protected:

		double Get_Current_Thread_Time( void ) const;

		// Scheduling interface, intended for derived classes
		virtual double Get_Reschedule_Time( void ) const;
		virtual double Get_Reschedule_Interval( void ) const;
		virtual bool Should_Reschedule( void ) const;

		// protected message handling (derived overridable/modifiable)
		virtual void Register_Message_Handlers( void );

		void Register_Handler( const std::type_info &message_type_info, const shared_ptr< IThreadMessageHandler > &handler );

		virtual void Handle_Shutdown_Thread_Request( const SThreadKey &key, const shared_ptr< const CShutdownThreadRequest > &message );

	private:

		friend class CThreadTaskBaseTester;
		friend class CThreadTaskBaseExaminer;

		// private accessors
		shared_ptr< CWriteOnlyThreadInterface > Get_Thread_Interface( const SThreadKey &key ) const;
		bool Is_Shutting_Down( void ) const;

		// Private message handling
		void Flush_Messages( void );

		void Service_Message_Frames( void );
		void Handle_Message( const SThreadKey &key, const shared_ptr< const IThreadMessage > &message );

		void Handle_Add_Write_Interface_Message( const SThreadKey &key, const shared_ptr< const CAddInterfaceMessage > &message );
		void Handle_Shutdown_Interface_Message( const SThreadKey &key, const shared_ptr< const CShutdownInterfaceMessage > &message );

		void Handle_Shutdown_Interfaces( void );

		// Type definitions
		typedef stdext::hash_map< SThreadKey, shared_ptr< CWriteOnlyThreadInterface >, SThreadKeyContainerHelper > InterfaceTable;
		typedef stdext::hash_map< SThreadKey, shared_ptr< CThreadMessageFrame >, SThreadKeyContainerHelper > FrameTableType;
		typedef stdext::hash_map< Loki::TypeInfo, shared_ptr< IThreadMessageHandler >, STypeInfoContainerHelper > ThreadMessageHandlerTableType;

		// Private Data
		// Simple state
		SThreadKey Key;

		EThreadState State;

		// Message frames
		FrameTableType PendingOutboundFrames;
		shared_ptr< CThreadMessageFrame > ManagerFrame;
		shared_ptr< CThreadMessageFrame > LogFrame;

		// Thread interfaces
		InterfaceTable Interfaces;
		shared_ptr< CWriteOnlyThreadInterface > ManagerInterface;
		shared_ptr< CWriteOnlyThreadInterface > LogInterface;

		shared_ptr< CReadOnlyThreadInterface > ReadInterface;

		std::set< SThreadKey, SThreadKeyContainerHelper > ShutdownInterfaces;

		// Timing
		double FirstServiceTimeSeconds;
		double CurrentTimeSeconds;

		// Misc
		ThreadMessageHandlerTableType MessageHandlers;

		scoped_ptr< CTaskScheduler > TaskScheduler;
};

#endif // THREAD_TASK_BASE_H
