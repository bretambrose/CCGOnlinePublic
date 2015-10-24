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

#include "StructuredExceptionInfo.h"

namespace IP
{
namespace Debug
{

CStackFrame::CStackFrame( void ) :
	Address( 0 ),
	FunctionName( L"" ),
	ModuleName( L"" ),
	FileName( L"" ),
	LineNumber( 0 )
{
}


CStackFrame::CStackFrame( const CStackFrame &rhs ) :
	Address( rhs.Address ),
	FunctionName( rhs.FunctionName ),
	ModuleName( rhs.ModuleName ),
	FileName( rhs.FileName ),
	LineNumber( rhs.LineNumber )
{
}


CStackFrame::CStackFrame( uint64_t address, const std::wstring &function_name, const std::wstring &module_name ) :
	Address( address ),
	FunctionName( function_name ),
	ModuleName( module_name ),
	FileName( L"" ),
	LineNumber( 0 )
{
}


CStackFrame::CStackFrame( uint64_t address, const std::wstring &function_name, const std::wstring &module_name, const std::wstring &file_name, uint64_t line_number ) :
	Address( address ),
	FunctionName( function_name ),
	ModuleName( module_name ),
	FileName( file_name ),
	LineNumber( line_number )
{
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


CStructuredExceptionInfo::CStructuredExceptionInfo( bool is_test_exception ) :
	ProcessID( 0 ),
	CallStack(),
	SymbolError( L"" ),
	ExceptionMessage( L"" ),
	IsTestException( is_test_exception )
{
}

} // namespace Debug
} // namespace IP