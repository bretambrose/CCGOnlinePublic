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

#pragma once

namespace IP
{
namespace Debug
{

// A class representing a single frame of the call stack
struct CStackFrame
{
	public:

		// Construction
		CStackFrame( void );
		CStackFrame( const CStackFrame &rhs );
		CStackFrame( uint64_t address, const std::wstring &function_name, const std::wstring &module_name );
		CStackFrame( uint64_t address, const std::wstring &function_name, const std::wstring &module_name, const std::wstring &file_name, uint64_t line_number );

		// Public interface
		// Accessors
		uint64_t Get_Address( void ) const { return Address; }
		const std::wstring &Get_Function_Name( void ) const { return FunctionName; }
		const std::wstring &Get_Module_Name( void ) const { return ModuleName; }
		const std::wstring &Get_File_Name( void ) const { return FileName; }
		uint64_t Get_Line_Number( void ) const { return LineNumber; }

	private:

		// Private data
		uint64_t Address;
		
		std::wstring FunctionName;
		std::wstring ModuleName;
		std::wstring FileName;

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
		const std::vector< CStackFrame > &Get_Call_Stack( void ) const { return CallStack; }

		const std::wstring &Get_Symbol_Error( void ) const { return SymbolError; }
		std::wstring &Get_Symbol_Error( void ) { return SymbolError; }
		void Set_Symbol_Error( const std::wstring &symbol_error ) { SymbolError = symbol_error; }

		const std::wstring &Get_Exception_Message( void ) const { return ExceptionMessage; }
		void Set_Exception_Message( const std::wstring &exception_message ) { ExceptionMessage = exception_message; }

		bool Is_Test_Exception( void ) const { return IsTestException; }

	private:

		// Private Data
		uint64_t ProcessID;

		std::vector< CStackFrame > CallStack;

		std::wstring ExceptionMessage;
		std::wstring SymbolError;

		bool IsTestException;
};

} // namespace Debug
} // namespace IP