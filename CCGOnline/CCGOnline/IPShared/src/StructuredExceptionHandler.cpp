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

#include <IPShared/StructuredExceptionHandler.h>

#include <IPCore/Debug/ExceptionHandler.h>
#include <IPCore/Debug/StructuredExceptionInfo.h>
#include <IPCore/File/FileSystem.h>
#include <IPCore/Memory/Stl/StringStream.h>
#include <IPCore/Process/Process.h>
#include <IPCore/System/Ids.h>
#include <IPShared/Logging/LogInterface.h>
#include <IPShared/Concurrency/ProcessExecutionContext.h>
#include <IPShared/Concurrency/ProcessStatics.h>
#include <IPShared/Concurrency/ProcessInterface.h>
#include <IPShared/Concurrency/ProcessConstants.h>
#include <IPShared/Concurrency/ProcessID.h>

#include <algorithm>
#include <fstream>
#include <iostream>


using namespace IP::Debug;
using namespace IP::Execution;
using namespace IP::Logging;

namespace IP
{
namespace Debug
{
namespace Exception
{
namespace StructuredExceptionHandler
{

// A sort function object for log file names that forces the exception file to be first so that when
// we iterate the files, the concatentated archive always has the exception at the top
struct SLogFileSorter
{
	public:
		
		bool operator()( const IP::String &lhs, const IP::String &rhs )
		{
			if ( lhs.size() > 13 && IP::String( lhs.c_str() + lhs.size() - 13, lhs.c_str() + lhs.size() ) == "Exception.txt" )
			{
				return true;
			}
			else if ( rhs.size() > 13 && IP::String( rhs.c_str() + rhs.size() - 13, rhs.c_str() + rhs.size() ) == "Exception.txt" )
			{
				return false;
			}
			else
			{
				return lhs < rhs;
			}
		}
};


static IP::String Get_Log_File_Prefix( void )
{
	IP::OStringStream prefix_string;
	prefix_string << CLogInterface::Get_Log_Path() << IP::Process::Get_Service_Name() << "_" << IP::Process::Get_Self_PID() << "_";

	return prefix_string.rdbuf()->str();
}

static void Archive_Logs( void )
{
	IP::OStringStream archive_name_string;
	archive_name_string << CLogInterface::Get_Archive_Path() << IP::Process::Get_Service_Name() << "_" << IP::Process::Get_Self_PID() << "_";
	archive_name_string << IP::Ids::Get_Semi_Unique_ID() << "_Crash.txt";

	IP::String archive_name = archive_name_string.rdbuf()->str();

	std::ofstream archive_file( archive_name.c_str(), std::ios_base::out | std::ios_base::trunc );

	IP::String log_pattern = Get_Log_File_Prefix() + "*.txt";

	IP::Vector< IP::String > file_names;
	IP::FileSystem::Enumerate_Matching_Files( log_pattern, file_names );

	// sort so that the exception file is first
	std::sort( file_names.begin(), file_names.end(), SLogFileSorter() );

	for ( uint32_t i = 0; i < file_names.size(); i++ )
	{
		IP::String input_log_name( CLogInterface::Get_Log_Path() + file_names[ i ] );
		std::ifstream log_file( input_log_name.c_str(), std::ios_base::in );

		archive_file << "File: " << file_names[ i ] << "\n\n";
		while( log_file.good() )
		{
			IP::String line;
			std::getline( log_file, line );

			archive_file << line << "\n";
		}

		archive_file << "\n---------------------------------------------------------------------------------------------------------------\n\n";
	}

	archive_file.close();
}

static void Write_Exception_File( const Exception::CStructuredExceptionInfo &shared_exception_info )
{
	IP::String file_name = Get_Log_File_Prefix() + "Exception.txt";

	std::ofstream exception_file( file_name.c_str(), std::ios_base::out | std::ios_base::trunc );

	exception_file << "******************** FATAL EXCEPTION ********************\n";
	exception_file << "Exception: " << shared_exception_info.Get_Exception_Message() << "\n";
	exception_file << "ConcurrentProcessID: " << std::hex << shared_exception_info.Get_Process_ID() << std::dec << "\n";
	exception_file << "\nCallstack:\n";
	
	// Iterate the call stack and write out an info line for each frame
	auto call_stack = shared_exception_info.Get_Call_Stack();
	for ( uint32_t i = 0; i < call_stack.size(); i++ )
	{
		const Exception::CStackFrame &frame = call_stack[ i ];
		
		exception_file << "  " << frame.Get_Module_Name() << "!" << frame.Get_Function_Name();
		if ( frame.Get_File_Name().size() > 0 )
		{
			exception_file << " (" << IP::FileSystem::Strip_Path( frame.Get_File_Name() ) << ", line " << frame.Get_Line_Number() << ")";
		}

		exception_file << "\n";
	}

	exception_file.close();
}

static void On_Structured_Exception_Callback( Exception::CStructuredExceptionInfo &shared_exception_info )
{
	// If the exception is in the scope of a thread task, record that fact
	EProcessID process_id = EProcessID::INVALID;
	IProcess *task = CProcessStatics::Get_Current_Process();
	if ( task != nullptr )
	{
		process_id = task->Get_ID();
	}

	shared_exception_info.Set_Process_ID( static_cast< uint64_t >( process_id ) );

	// Flush all logging info to disk (by using a null context); global log mutex serializes access
	CProcessExecutionContext context( nullptr, 0.0 );
	CLogInterface::Service_Logging( context );

	// Write a file containing the exception details
	Write_Exception_File( shared_exception_info );

	// Concat all log files and the exception information file into one archive file
	Archive_Logs();
}

void Initialize( void )
{
	Exception::Initialize( Exception::DExceptionHandler( On_Structured_Exception_Callback ) );
}

void Initialize( const Exception::DExceptionHandler &handler )
{
	Exception::Initialize( handler );
}

void Shutdown( void )
{
	Exception::Shutdown();
}

} // namespace StructuredExceptionHandler
} // namespace Exception
} // namespace Debug
} // namespace IP
