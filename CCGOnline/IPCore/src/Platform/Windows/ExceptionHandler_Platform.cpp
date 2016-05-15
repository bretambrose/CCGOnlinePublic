/**********************************************************************************************************************

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


#include <IPCore/Debug/ExceptionHandler.h>

#include <IPCore/Debug/StructuredExceptionInfo.h>
#include <IPCore/Memory/Stl/StringStream.h>
#include <IPCore/Platform/Windows/Error.h>
#include <IPCore/System/WindowsWrapper.h>

#include <iostream>
#include <mutex>

#include <DbgHelp.h>

static IP::Debug::Exception::DExceptionHandler Handler;
static std::mutex ExceptionLock;
static bool SymbolsLoaded = false;
static bool Initialized = false;

static void On_Exception( IP::Debug::Exception::CStructuredExceptionInfo &shared_exception_info )
{
	if ( Handler )
	{
		Handler( shared_exception_info );
	}
}

static bool Load_Symbols( IP::String &error_message )
{
	if ( SymbolsLoaded )
	{
		return true;
	}

	uint32_t symbol_options = ::SymGetOptions();
	symbol_options |= SYMOPT_LOAD_LINES;
	::SymSetOptions( symbol_options );

	if ( !::SymInitialize( ::GetCurrentProcess(), nullptr, TRUE ) )
	{
		error_message = IP::Error::Format_OS_Error_Message( ::GetLastError() );
		return false;
	}

	SymbolsLoaded = true;
	return true;
}


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

static void Capture_Stack_Trace( __in_opt CONST PCONTEXT initial_context, IP::Debug::Exception::CStructuredExceptionInfo &info )
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
			info.Set_Symbol_Error( IP::Error::Format_OS_Error_Message( ::GetLastError() ) );
			break;
		}

		uint64_t address = stack_frame.AddrPC.Offset;
		if ( address == 0 )
		{
			break;
		}

		// get the function name for the current stack frame
		static const uint32_t MAX_SYMBOL_NAME_LENGTH = 512;
		static const uint32_t SYMBOL_INFO_EXTENDED_LENGTH = sizeof( SYMBOL_INFOW ) + MAX_SYMBOL_NAME_LENGTH * sizeof( TCHAR );

		char buffer[ SYMBOL_INFO_EXTENDED_LENGTH ];
		::ZeroMemory( buffer, SYMBOL_INFO_EXTENDED_LENGTH );

		PSYMBOL_INFO p_symbol = reinterpret_cast< PSYMBOL_INFO >( buffer );
		p_symbol->SizeOfStruct = sizeof( SYMBOL_INFO );
		p_symbol->MaxNameLen = MAX_SYMBOL_NAME_LENGTH;
	
		success = ::SymFromAddr( current_process, address, nullptr, p_symbol ) == TRUE;
		if ( !success )
		{
			info.Set_Symbol_Error( IP::Error::Format_OS_Error_Message( ::GetLastError() ) );
			break;
		}

		IP::String symbol_name( p_symbol->Name, p_symbol->Name + p_symbol->NameLen );

		// get the module name for the current stack frame
		IMAGEHLP_MODULE64 module_info;
		::ZeroMemory( &module_info, sizeof( IMAGEHLP_MODULE64 ) );
		module_info.SizeOfStruct = sizeof( IMAGEHLP_MODULE64 );
		if ( ::SymGetModuleInfo64( current_process, address, &module_info ) == FALSE )
		{
			info.Set_Symbol_Error( IP::Error::Format_OS_Error_Message( ::GetLastError() ) );
			break;
		}

		IP::String module_name( module_info.ModuleName );

		// get the file name and line number for the current stack frame
		DWORD displacement = 0;

		IMAGEHLP_LINE64 line_info;
		::ZeroMemory( &line_info, sizeof( IMAGEHLP_LINE64 ) );
		line_info.SizeOfStruct = sizeof( IMAGEHLP_LINE64 );

		// add all the gathered info to the exception data as a stack frame entry
		bool file_success = ::SymGetLineFromAddr64( current_process, address, &displacement, &line_info ) == TRUE;
		if ( file_success )
		{
			info.Add_Frame( IP::Debug::Exception::CStackFrame( address, symbol_name, module_name, IP::String( line_info.FileName ), line_info.LineNumber ) );
		}
		else
		{
			info.Add_Frame( IP::Debug::Exception::CStackFrame( address, symbol_name, module_name ) );
		}
	}
}

#ifdef _M_IX86

#pragma warning( pop )
#pragma optimize( "g", on )

#endif

static IP::String Convert_Exception_Code_To_Message( uint32_t exception_code )
{
	switch ( exception_code )
	{
		case EXCEPTION_ACCESS_VIOLATION:
			return IP::String( "Access Violation" );
			
		case EXCEPTION_FLT_DIVIDE_BY_ZERO:
			return IP::String( "FP Divide By Zero" );

		case EXCEPTION_FLT_INVALID_OPERATION:
			return IP::String( "FP Invalid Op" );

		case EXCEPTION_FLT_OVERFLOW:
			return IP::String( "FP Overflow" );

		case EXCEPTION_FLT_UNDERFLOW:
			return IP::String( "FP Underflow" );

		case EXCEPTION_ILLEGAL_INSTRUCTION:
			return IP::String( "Illegal Instruction" );

		case EXCEPTION_INT_DIVIDE_BY_ZERO:
			return IP::String( "Divide By Zero" );

		case EXCEPTION_NONCONTINUABLE_EXCEPTION:
			return IP::String( "Noncontinuable Exception" );

		case EXCEPTION_PRIV_INSTRUCTION:
			return IP::String( "Privileged Instruction" );

		case EXCEPTION_STACK_OVERFLOW:
			return IP::String( "Stack Overflow" );

		default:
		{
			IP::StringStream error_string;
			error_string << "Unknown Exception Code: " << exception_code;
			return error_string.str();
		}
	}
}


static LONG WINAPI WindowsExceptionHandler( struct _EXCEPTION_POINTERS *windows_exception_info )
{
	std::lock_guard< std::mutex > exception_lock( ExceptionLock );
		
	uint32_t exception_code = windows_exception_info->ExceptionRecord->ExceptionCode;
	IP::Debug::Exception::CStructuredExceptionInfo shared_exception_info( exception_code == 1 );
	shared_exception_info.Set_Exception_Message( ::Convert_Exception_Code_To_Message( exception_code ) );

	if ( Load_Symbols( shared_exception_info.Get_Symbol_Error() ) )
	{
		::Capture_Stack_Trace( windows_exception_info->ContextRecord, shared_exception_info );
	}

	On_Exception( shared_exception_info );

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

namespace IP
{
namespace Debug
{
namespace Exception
{

void Initialize( const DExceptionHandler &handler )
{
	std::lock_guard< std::mutex > exception_lock( ExceptionLock );

	if ( Initialized )
	{
		return;
	}

	Handler = handler;
	::AddVectoredExceptionHandler( 1, WindowsExceptionHandler );

	Initialized = true;
}


void Shutdown( void )
{
	std::lock_guard< std::mutex > exception_lock( ExceptionLock );

	if ( !Initialized )
	{
		return;
	}

	Handler = DExceptionHandler();
	::RemoveVectoredExceptionHandler( WindowsExceptionHandler );

	if ( SymbolsLoaded )
	{
		::SymCleanup( ::GetCurrentProcess() );
		SymbolsLoaded = false;
	}

	Initialized = false;
}




} // namespace Exception
} // namespace Debug
} // namespace IP
