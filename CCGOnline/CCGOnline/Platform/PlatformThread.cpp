/**********************************************************************************************************************

	PlatformThread.cpp
		A simple component that wraps OS threads

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

#include "PlatformThread.h"

// Our internal platform-independent thread execution context
struct STrueThreadExecutionContext
{
	public:

		STrueThreadExecutionContext( void ) :
			ExecutionFunction(),
			RunContext( nullptr )
		{
		}

		void Initialize( const ThreadExecutionFunctionType &execution_function, void *run_context )
		{
			ExecutionFunction = execution_function;
			RunContext = run_context;
		}

		const ThreadExecutionFunctionType &Get_Execution_Function( void ) const { return ExecutionFunction; }
		void *Get_Run_Context( void ) const { return RunContext; }

	private:

		ThreadExecutionFunctionType ExecutionFunction;
		void *RunContext;
};

// An interface that makes explicit and clear what functions need to be supported by an OS-specific thread wrapper
class IPlatformThread
{
	public:

		virtual ~IPlatformThread() {}

		virtual void Launch_Thread( uint64 stack_size, const ThreadExecutionFunctionType &execution_function, void *run_context ) = 0;
		virtual void Shutdown_Thread( void ) = 0;
		virtual bool Is_Valid( void ) const = 0;
		
};

#ifdef WIN32

#include <Process.h>

// Wraps OS-specific thread functionality
class CPlatformThreadImpl : public IPlatformThread
{
	public:

		typedef IPlatformThread BASECLASS;

		CPlatformThreadImpl( void );

		virtual void Launch_Thread( uint64 stack_size, const ThreadExecutionFunctionType &execution_function, void *run_context );
		virtual void Shutdown_Thread( void );
		virtual bool Is_Valid( void ) const { return ThreadHandle != NULL; }

	private:

		static uint32 WINAPI Run_Thread( LPVOID thread_param );

		HANDLE ThreadHandle;

		STrueThreadExecutionContext TranslationContext;
};

/**********************************************************************************************************************
	CPlatformThreadImpl::CPlatformThreadImpl -- default constructor

**********************************************************************************************************************/
CPlatformThreadImpl::CPlatformThreadImpl( void ) :
	ThreadHandle( NULL ),
	TranslationContext()
{
}

/**********************************************************************************************************************
	CPlatformThreadImpl::Launch_Thread -- creates and starts a new Windows thread

		stack_size -- desired size of thread's user stack, 0 for default (1 MB)
		execution_function -- what function the thread should run
		run_context -- parameter to the execution function

**********************************************************************************************************************/
void CPlatformThreadImpl::Launch_Thread( uint64 stack_size, const ThreadExecutionFunctionType &execution_function, void *run_context )
{
	FATAL_ASSERT( ThreadHandle == NULL );

	TranslationContext.Initialize( execution_function, run_context );

	ThreadHandle = reinterpret_cast< HANDLE >( 
		::_beginthreadex( NULL, static_cast< uint32 >( stack_size ), CPlatformThreadImpl::Run_Thread, &TranslationContext, 0, NULL ) );

	FATAL_ASSERT( ThreadHandle != NULL );
}

/**********************************************************************************************************************
	CPlatformThreadImpl::Shutdown_Thread -- cleans up a Windows thread

**********************************************************************************************************************/
void CPlatformThreadImpl::Shutdown_Thread( void )
{
	if ( !Is_Valid() )
	{
		return;
	}

	DWORD exit_code = 0;
	::GetExitCodeThread( ThreadHandle, &exit_code );
			
	if ( exit_code == STILL_ACTIVE )
	{
		::TerminateThread( ThreadHandle, 1 );
	}

	::CloseHandle( ThreadHandle );
	ThreadHandle = NULL;
}

/**********************************************************************************************************************
	CPlatformThreadImpl::Run_Thread -- Windows compatible wrapper function around the thread's actual execution invocation

		thread_param -- execution context needed to reconstruct the actual function invocation

		Returns: 0

**********************************************************************************************************************/
uint32 WINAPI CPlatformThreadImpl::Run_Thread( LPVOID thread_param )
{
	STrueThreadExecutionContext *translation_context = static_cast< STrueThreadExecutionContext * >( thread_param );

	( translation_context->Get_Execution_Function() )( translation_context->Get_Run_Context() );

	return 0;
}

#endif // WIN32

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/**********************************************************************************************************************
	CPlatformThread::CPlatformThread -- default constructor

**********************************************************************************************************************/
CPlatformThread::CPlatformThread( void ) :
	ThreadImpl( new CPlatformThreadImpl )
{
}

/**********************************************************************************************************************
	CPlatformThread::~CPlatformThread -- destructor

**********************************************************************************************************************/
CPlatformThread::~CPlatformThread()
{
	Shutdown();
}

/**********************************************************************************************************************
	CPlatformThread::Create_And_Run -- creates and starts a new platform thread

		stack_size -- desired size of thread's user stack, 0 for default (1 MB)
		execution_function -- what function the thread should run
		run_context -- parameter to the execution function

**********************************************************************************************************************/
void CPlatformThread::Create_And_Run( uint64 stack_size, const ThreadExecutionFunctionType &execution_function, void *run_context )
{
	FATAL_ASSERT( !ThreadImpl->Is_Valid() );

	ThreadImpl->Launch_Thread( stack_size, execution_function, run_context );
}

/**********************************************************************************************************************
	CPlatformThread::Shutdown -- shuts down a platform thread

**********************************************************************************************************************/
void CPlatformThread::Shutdown( void )
{
	ThreadImpl->Shutdown_Thread();
}


