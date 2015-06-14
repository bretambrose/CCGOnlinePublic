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

#include "stdafx.h"

#include "StructuredExceptionHandler.h"

#include <fstream>
#include <sstream>
#include <iostream>
#include "IPPlatform/PlatformExceptionHandler.h"
#include "Logging/LogInterface.h"
#include "Concurrency/ProcessExecutionContext.h"
#include "Concurrency/ProcessStatics.h"
#include "Concurrency/ProcessInterface.h"
#include "Concurrency/ProcessConstants.h"
#include "Concurrency/ProcessID.h"
#include "IPPlatform/StructuredExceptionInfo.h"
#include "IPPlatform/PlatformFileSystem.h"
#include "IPPlatform/PlatformMisc.h"
#include "IPPlatform/PlatformProcess.h"


void CStructuredExceptionHandler::Initialize( void )
{
	CPlatformExceptionHandler::Initialize( DExceptionHandler( CStructuredExceptionHandler::On_Structured_Exception_Callback ) );
}


void CStructuredExceptionHandler::Initialize( const DExceptionHandler &handler )
{
	CPlatformExceptionHandler::Initialize( handler );
}


void CStructuredExceptionHandler::Shutdown( void )
{
	CPlatformExceptionHandler::Shutdown();
}


void CStructuredExceptionHandler::On_Structured_Exception_Callback( CStructuredExceptionInfo &shared_exception_info )
{
	// If the exception is in the scope of a thread task, record that fact
	EProcessID::Enum process_id = EProcessID::INVALID;
	IProcess *task = CProcessStatics::Get_Current_Process();
	if ( task != nullptr )
	{
		process_id = task->Get_ID();
	}

	shared_exception_info.Set_Process_ID( process_id );

	// Flush all logging info to disk (by using a null context); global log mutex serializes access
	CProcessExecutionContext context( nullptr, 0.0 );
	CLogInterface::Service_Logging( context );

	// Write a file containing the exception details
	Write_Exception_File( shared_exception_info );

	// Concat all log files and the exception information file into one archive file
	Archive_Logs();
}


void CStructuredExceptionHandler::Write_Exception_File( const CStructuredExceptionInfo &shared_exception_info )
{
	std::wstring file_name = Get_Log_File_Prefix() + L"Exception.txt";

	std::basic_ofstream< wchar_t > exception_file( file_name.c_str(), std::ios_base::out | std::ios_base::trunc );

	exception_file << L"******************** FATAL EXCEPTION ********************\n";
	exception_file << L"Exception: " << shared_exception_info.Get_Exception_Message() << L"\n";
	exception_file << L"ConcurrentProcessID: " << std::hex << shared_exception_info.Get_Process_ID() << std::dec << L"\n";
	exception_file << L"\nCallstack:\n";
	
	// Iterate the call stack and write out an info line for each frame
	auto call_stack = shared_exception_info.Get_Call_Stack();
	for ( uint32_t i = 0; i < call_stack.size(); i++ )
	{
		const CStackFrame &frame = call_stack[ i ];
		
		exception_file << L"  " << frame.Get_Module_Name() << L"!" << frame.Get_Function_Name();
		if ( frame.Get_File_Name().size() > 0 )
		{
			exception_file << L" (" << NPlatform::Strip_Path( frame.Get_File_Name() ) << L", line " << frame.Get_Line_Number() << L")";
		}

		exception_file << L"\n";
	}

	exception_file.close();
}

// A sort function object for log file names that forces the exception file to be first so that when
// we iterate the files, the concatentated archive always has the exception at the top
struct SLogFileSorter
{
	public:
		
		bool operator()( const std::wstring &lhs, const std::wstring &rhs )
		{
			if ( lhs.size() > 13 && std::wstring( lhs.c_str() + lhs.size() - 13, lhs.c_str() + lhs.size() ) == L"Exception.txt" )
			{
				return true;
			}
			else if ( rhs.size() > 13 && std::wstring( rhs.c_str() + rhs.size() - 13, rhs.c_str() + rhs.size() ) == L"Exception.txt" )
			{
				return false;
			}
			else
			{
				return lhs < rhs;
			}
		}
};


void CStructuredExceptionHandler::Archive_Logs( void )
{
	std::basic_ostringstream< wchar_t > archive_name_string;
	archive_name_string << CLogInterface::Get_Archive_Path() << NPlatform::Get_Service_Name() << L"_" << NPlatform::Get_Self_PID() << L"_";
	archive_name_string << NPlatform::Get_Semi_Unique_ID() << L"_Crash.txt";

	std::wstring archive_name = archive_name_string.rdbuf()->str();

	std::basic_ofstream< wchar_t > archive_file( archive_name.c_str(), std::ios_base::out | std::ios_base::trunc );

	std::wstring log_pattern = Get_Log_File_Prefix() + L"*.txt";

	std::vector< std::wstring > file_names;
	NPlatform::Enumerate_Matching_Files( log_pattern, file_names );

	// sort so that the exception file is first
	std::sort( file_names.begin(), file_names.end(), SLogFileSorter() );

	for ( uint32_t i = 0; i < file_names.size(); i++ )
	{
		std::basic_ifstream< wchar_t > log_file( CLogInterface::Get_Log_Path() + file_names[ i ], std::ios_base::in );

		archive_file << L"File: " << file_names[ i ] << L"\n\n";
		while( log_file.good() )
		{
			std::wstring line;
			std::getline( log_file, line );

			archive_file << line << L"\n";
		}

		archive_file << L"\n---------------------------------------------------------------------------------------------------------------\n\n";
	}

	archive_file.close();
}


std::wstring CStructuredExceptionHandler::Get_Log_File_Prefix( void )
{
	std::basic_ostringstream< wchar_t > prefix_string;
	prefix_string << CLogInterface::Get_Log_Path() << NPlatform::Get_Service_Name() << L"_" << NPlatform::Get_Self_PID() << L"_";

	return prefix_string.rdbuf()->str();
}



