/**********************************************************************************************************************

	[Placeholder for eventual source license]

	StructuredExceptionInfo.h
		A component that holds data about an exception.  OS-specific logic extracts call stack and context information
			and adds it to this OS-agnostic object.

	(c) Copyright 2011, Bret Ambrose.  All rights reserved.

**********************************************************************************************************************/

#ifndef STRUCTURED_EXCEPTION_INFO_H
#define STRUCTURED_EXCEPTION_INFO_H

// A class representing a single frame of the call stack
struct CStackFrame
{
	public:

		// Construction
		CStackFrame( void );
		CStackFrame( const CStackFrame &rhs );
		CStackFrame( uint64 address, const std::wstring &function_name, const std::wstring &module_name );
		CStackFrame( uint64 address, const std::wstring &function_name, const std::wstring &module_name, const std::wstring &file_name, uint64 line_number );

		// Public interface
		// Accessors
		uint64 Get_Address( void ) const { return Address; }
		const std::wstring &Get_Function_Name( void ) const { return FunctionName; }
		const std::wstring &Get_Module_Name( void ) const { return ModuleName; }
		const std::wstring &Get_File_Name( void ) const { return FileName; }
		uint64 Get_Line_Number( void ) const { return LineNumber; }

	private:

		// Private data
		uint64 Address;
		
		std::wstring FunctionName;
		std::wstring ModuleName;
		std::wstring FileName;

		uint64 LineNumber;

};

// A class that represents the context of a single exception event
class CStructuredExceptionInfo
{
	public:

		// Construction
		CStructuredExceptionInfo( bool is_test_exception );

		// Public interface
		// Accessors
		uint64 Get_Thread_Key( void ) const { return ThreadKey; }
		void Set_Thread_Key( uint64 thread_key ) { ThreadKey = thread_key; }

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
		uint64 ThreadKey;

		std::vector< CStackFrame > CallStack;

		std::wstring ExceptionMessage;
		std::wstring SymbolError;

		bool IsTestException;
};

#endif // STRUCTURED_EXCEPTION_INFO_H