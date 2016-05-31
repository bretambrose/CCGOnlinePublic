/*********************************************************************************************************************

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

#pragma once

#include <IPCore/Memory/Stl/String.h>
#include <IPCore/Memory/Stl/Vector.h>

namespace IP
{
namespace Debug
{
namespace Exception
{

// A class representing a single frame of the call stack
struct CStackFrame
{
	public:

		// Construction
		CStackFrame( void );
		CStackFrame( const CStackFrame &rhs );
		CStackFrame( uint64_t address, const IP::String &function_name, const IP::String &module_name );
		CStackFrame( uint64_t address, const IP::String &function_name, const IP::String &module_name, const IP::String &file_name, uint64_t line_number );

		// Public interface
		// Accessors
		uint64_t Get_Address( void ) const { return Address; }
		const IP::String &Get_Function_Name( void ) const { return FunctionName; }
		const IP::String &Get_Module_Name( void ) const { return ModuleName; }
		const IP::String &Get_File_Name( void ) const { return FileName; }
		uint64_t Get_Line_Number( void ) const { return LineNumber; }

	private:

		// Private data
		uint64_t Address;
		
		IP::String FunctionName;
		IP::String ModuleName;
		IP::String FileName;

		uint64_t LineNumber;

};

// A class that represents the context of a single exception event
class CStructuredExceptionInfo
{
	public:

		// Construction
		CStructuredExceptionInfo( bool is_test_exception );

		// Public interface
		// Accessors
		uint64_t Get_Process_ID( void ) const { return ProcessID; }
		void Set_Process_ID( uint64_t process_id ) { ProcessID = process_id; }

		void Add_Frame( const CStackFrame &frame ) { CallStack.push_back( frame ); }
		const IP::Vector< CStackFrame > &Get_Call_Stack( void ) const { return CallStack; }

		const IP::String &Get_Symbol_Error( void ) const { return SymbolError; }
		IP::String &Get_Symbol_Error( void ) { return SymbolError; }
		void Set_Symbol_Error( const IP::String &symbol_error ) { SymbolError = symbol_error; }

		const IP::String &Get_Exception_Message( void ) const { return ExceptionMessage; }
		void Set_Exception_Message( const IP::String &exception_message ) { ExceptionMessage = exception_message; }

		bool Is_Test_Exception( void ) const { return IsTestException; }

	private:

		// Private Data
		uint64_t ProcessID;

		IP::Vector< CStackFrame > CallStack;

		IP::String ExceptionMessage;
		IP::String SymbolError;

		bool IsTestException;
};

} // namespace Exception
} // namespace Debug
} // namespace IP