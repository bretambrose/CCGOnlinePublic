/**********************************************************************************************************************

	PlatformExceptionHandler.cpp
		A component that wraps OS-specific exception handling logic, including generating a stack trace

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

#include <sstream>
#include <iostream>
#include <DbgHelp.h>

#include "PlatformExceptionHandler.h"
#include "PlatformMisc.h"
#include "StructuredExceptionInfo.h"
#include "SynchronizationPrimitives/PlatformMutex.h"

#ifdef WIN32

/*
	Adapted from Johannes Passing's tutorial on stack walking in windows: http://jpassing.com/2008/03/12/walking-the-stack-of-the-current-thread/
*/

#ifdef _M_IX86

//
// Disable global optimization and ignore /GS warning caused by
// inline assembly.
//
#pragma optimize( "g", off )
#pragma warning( push )
#pragma warning( disable : 4748 )

#endif

static void Capture_Stack_Trace( __in_opt CONST PCONTEXT initial_context, CStructuredExceptionInfo &info )
{
	DWORD machine_type;
	CONTEXT local_context;
	STACKFRAME64 stack_frame;
	HANDLE current_process = ::GetCurrentProcess();

	if ( initial_context == nullptr )
	{
		// Use current context.
		//
		// N.B. GetThreadContext cannot be used on the current thread.
		// Capture own context - on i386, there is no API for that.
#ifdef _M_IX86
		::ZeroMemory( &local_context, sizeof( CONTEXT ) );
		local_context.ContextFlags = CONTEXT_CONTROL;

		// Those three registers are enough.
		__asm
		{
			Label:
				mov [local_context.Ebp], ebp;
				mov [local_context.Esp], esp;
				mov eax, [Label];
				mov [local_context.Eip], eax;
		}
#else
		RtlCaptureContext( &local_context );
#endif
	}
	else
	{
		::CopyMemory( &local_context, initial_context, sizeof( CONTEXT ) );
	}

	// Set up stack frame.
	::ZeroMemory( &stack_frame, sizeof( STACKFRAME64 ) );
#ifdef _M_IX86
	machine_type                 = IMAGE_FILE_MACHINE_I386;
	stack_frame.AddrPC.Offset    = local_context.Eip;
	stack_frame.AddrPC.Mode      = AddrModeFlat;
	stack_frame.AddrFrame.Offset = local_context.Ebp;
	stack_frame.AddrFrame.Mode   = AddrModeFlat;
	stack_frame.AddrStack.Offset = local_context.Esp;
	stack_frame.AddrStack.Mode   = AddrModeFlat;
#elif _M_X64
	machine_type                 = IMAGE_FILE_MACHINE_AMD64;
	stack_frame.AddrPC.Offset    = local_context.Rip;
	stack_frame.AddrPC.Mode      = AddrModeFlat;
	stack_frame.AddrFrame.Offset = local_context.Rsp;
	stack_frame.AddrFrame.Mode   = AddrModeFlat;
	stack_frame.AddrStack.Offset = local_context.Rsp;
	stack_frame.AddrStack.Mode   = AddrModeFlat;
#elif _M_IA64
	machine_type                 = IMAGE_FILE_MACHINE_IA64;
	stack_frame.AddrPC.Offset    = local_context.StIIP;
	stack_frame.AddrPC.Mode      = AddrModeFlat;
	stack_frame.AddrFrame.Offset = local_context.IntSp;
	stack_frame.AddrFrame.Mode   = AddrModeFlat;
	stack_frame.AddrBStore.Offset= local_context.RsBSP;
	stack_frame.AddrBStore.Mode  = AddrModeFlat;
	stack_frame.AddrStack.Offset = local_context.IntSp;
	stack_frame.AddrStack.Mode   = AddrModeFlat;
#else
	#error "Unsupported platform"
#endif

	// Note that the code assumes that
	// SymInitialize( GetCurrentProcess(), NULL, TRUE ) has
	// already been called.

	bool success = true;
	while ( success )
	{
		// get the current stack frame
		PCONTEXT walk_context = ( machine_type == IMAGE_FILE_MACHINE_I386 ) ? nullptr : &local_context;
		success = ::StackWalk64( machine_type, current_process, GetCurrentThread(), &stack_frame, walk_context, nullptr, SymFunctionTableAccess64, SymGetModuleBase64, NULL ) == TRUE;
		if ( !success )
		{
			info.Set_Symbol_Error( NPlatform::Format_OS_Error_Message( ::GetLastError() ) );
			break;
		}

		uint64 address = stack_frame.AddrPC.Offset;
		if ( address == 0 )
		{
			break;
		}

		// get the function name for the current stack frame
		static const uint32 MAX_SYMBOL_NAME_LENGTH = 512;
		static const uint32 SYMBOL_INFO_EXTENDED_LENGTH = sizeof( SYMBOL_INFOW ) + MAX_SYMBOL_NAME_LENGTH * sizeof( TCHAR );

		char buffer[ SYMBOL_INFO_EXTENDED_LENGTH ];
		::ZeroMemory( buffer, SYMBOL_INFO_EXTENDED_LENGTH );

		PSYMBOL_INFOW p_symbol = reinterpret_cast< PSYMBOL_INFOW >( buffer );
		p_symbol->SizeOfStruct = sizeof( SYMBOL_INFOW );
		p_symbol->MaxNameLen = MAX_SYMBOL_NAME_LENGTH;
	
		success = ::SymFromAddrW( current_process, address, nullptr, p_symbol ) == TRUE;
		if ( !success )
		{
			info.Set_Symbol_Error( NPlatform::Format_OS_Error_Message( ::GetLastError() ) );
			break;
		}

		std::wstring symbol_name( p_symbol->Name, p_symbol->Name + p_symbol->NameLen );

		// get the module name for the current stack frame
		IMAGEHLP_MODULEW64 module_info;
		::ZeroMemory( &module_info, sizeof( IMAGEHLP_MODULEW64 ) );
		module_info.SizeOfStruct = sizeof( IMAGEHLP_MODULEW64 );
		if ( ::SymGetModuleInfoW64( current_process, address, &module_info ) == FALSE )
		{
			info.Set_Symbol_Error( NPlatform::Format_OS_Error_Message( ::GetLastError() ) );
			break;
		}

		std::wstring module_name( module_info.ModuleName );

		// get the file name and line number for the current stack frame
		DWORD displacement = 0;

		IMAGEHLP_LINEW64 line_info;
		::ZeroMemory( &line_info, sizeof( IMAGEHLP_LINE64 ) );
		line_info.SizeOfStruct = sizeof( IMAGEHLP_LINE64 );

		// add all the gathered info to the exception data as a stack frame entry
		bool file_success = ::SymGetLineFromAddrW64( current_process, address, &displacement, &line_info ) == TRUE;
		if ( file_success )
		{
			info.Add_Frame( CStackFrame( address, symbol_name, module_name, std::wstring( line_info.FileName ), line_info.LineNumber ) );
		}
		else
		{
			info.Add_Frame( CStackFrame( address, symbol_name, module_name ) );
		}
	}
}

#ifdef _M_IX86

#pragma warning( pop )
#pragma optimize( "g", on )

#endif

/**********************************************************************************************************************
	Convert_Exception_Code_To_Message - Converts a Win32 exception code to a descriptive string

		exception_code -- the exception code to describe

		Returns: string describing the exception

**********************************************************************************************************************/
static std::wstring Convert_Exception_Code_To_Message( uint32 exception_code )
{
	switch ( exception_code )
	{
		case EXCEPTION_ACCESS_VIOLATION:
			return std::wstring( L"Access Violation" );
			
		case EXCEPTION_FLT_DIVIDE_BY_ZERO:
			return std::wstring( L"FP Divide By Zero" );

		case EXCEPTION_FLT_INVALID_OPERATION:
			return std::wstring( L"FP Invalid Op" );

		case EXCEPTION_FLT_OVERFLOW:
			return std::wstring( L"FP Overflow" );

		case EXCEPTION_FLT_UNDERFLOW:
			return std::wstring( L"FP Underflow" );

		case EXCEPTION_ILLEGAL_INSTRUCTION:
			return std::wstring( L"Illegal Instruction" );

		case EXCEPTION_INT_DIVIDE_BY_ZERO:
			return std::wstring( L"Divide By Zero" );

		case EXCEPTION_NONCONTINUABLE_EXCEPTION:
			return std::wstring( L"Noncontinuable Exception" );

		case EXCEPTION_PRIV_INSTRUCTION:
			return std::wstring( L"Privileged Instruction" );

		case EXCEPTION_STACK_OVERFLOW:
			return std::wstring( L"Stack Overflow" );

		default:
		{
			std::basic_ostringstream< wchar_t > error_string;
			error_string << L"Unknown Exception Code: " << exception_code;
			return error_string.rdbuf()->str();
		}
	}
}

/**********************************************************************************************************************
	WindowsExceptionHandler -- the top-level structured exception handler for our programs

		windows_exception_info -- windows-specific contextual information about the exception

		Returns: a code telling windows to either look for another handler or try and continue execution

**********************************************************************************************************************/
static LONG WINAPI WindowsExceptionHandler( struct _EXCEPTION_POINTERS *windows_exception_info )
{
	CSimplePlatformMutexLocker exception_lock( CPlatformExceptionHandler::Get_Lock() );
		
	uint32 exception_code = windows_exception_info->ExceptionRecord->ExceptionCode;
	CStructuredExceptionInfo shared_exception_info( exception_code == 1 );
	shared_exception_info.Set_Exception_Message( ::Convert_Exception_Code_To_Message( exception_code ) );

	if ( CPlatformExceptionHandler::Load_Symbols( shared_exception_info.Get_Symbol_Error() ) )
	{
		::Capture_Stack_Trace( windows_exception_info->ContextRecord, shared_exception_info );
	}

	CPlatformExceptionHandler::On_Exception( shared_exception_info );

	if ( shared_exception_info.Is_Test_Exception() )
	{
		return EXCEPTION_CONTINUE_EXECUTION;
	}
	else
	{
		return EXCEPTION_CONTINUE_SEARCH;
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

DExceptionHandler CPlatformExceptionHandler::Handler;
ISimplePlatformMutex *CPlatformExceptionHandler::ExceptionLock = nullptr;
bool CPlatformExceptionHandler::SymbolsLoaded = false;
bool CPlatformExceptionHandler::Initialized = false;

/**********************************************************************************************************************
	CPlatformExceptionHandler::Initialize -- Initializes the platform-specific exception handling mechanism

		handler -- a platform-agnostic exception handler to call once all information has been gathered

**********************************************************************************************************************/
void CPlatformExceptionHandler::Initialize( const DExceptionHandler &handler )
{
	FATAL_ASSERT( !Initialized );

	ExceptionLock = NPlatform::Create_Simple_Mutex();

	Handler = handler;
	::AddVectoredExceptionHandler( 1, WindowsExceptionHandler );

	Initialized = true;
}

/**********************************************************************************************************************
	CPlatformExceptionHandler::Shutdown -- shuts down and cleans up the platform-specific exception handling mechanism

**********************************************************************************************************************/
void CPlatformExceptionHandler::Shutdown( void )
{
	if ( !Initialized )
	{
		return;
	}

	Handler = DExceptionHandler();
	::RemoveVectoredExceptionHandler( WindowsExceptionHandler );

	delete ExceptionLock;
	ExceptionLock = nullptr;

	if ( SymbolsLoaded )
	{
		::SymCleanup( ::GetCurrentProcess() );
		SymbolsLoaded = false;
	}

	Initialized = false;
}

/**********************************************************************************************************************
	CPlatformExceptionHandler::On_Exception -- forwards the now-agnostic exception info to the agnostic exception
		handler

		shared_exception_info -- OS-neutral exception information

**********************************************************************************************************************/
void CPlatformExceptionHandler::On_Exception( CStructuredExceptionInfo &shared_exception_info )
{
	if ( !Handler.empty() )
	{
		Handler( shared_exception_info );
	}
}

/**********************************************************************************************************************
	CPlatformExceptionHandler::Load_Symbols -- loads the symbols associated with this process, only done when there's
		an exception

		error_message -- output parameter describing any errors that occurred during the loading process

		Returns: success/failure

**********************************************************************************************************************/
bool CPlatformExceptionHandler::Load_Symbols( std::wstring &error_message )
{
	if ( SymbolsLoaded )
	{
		return true;
	}

	uint32 symbol_options = ::SymGetOptions();
	symbol_options |= SYMOPT_LOAD_LINES;
	::SymSetOptions( symbol_options );

	if ( !::SymInitializeW( ::GetCurrentProcess(), nullptr, TRUE ) )
	{
		error_message = NPlatform::Format_OS_Error_Message( ::GetLastError() );
		return false;
	}

	SymbolsLoaded = true;
	return true;
}

#endif // WIN32