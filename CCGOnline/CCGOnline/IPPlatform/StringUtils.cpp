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

#include "StringUtils.h"

namespace IP
{
namespace String
{

void String_To_WideString( const std::string &source, std::wstring &target )
{
	size_t buffer_length = source.size() * 2 + 2;
	size_t buffer_word_length = buffer_length * sizeof( wchar_t ) / sizeof( uint32_t );
	wchar_t *target_buffer = new wchar_t[ buffer_length ];
	size_t bytes_written = 0;

	mbstowcs_s( &bytes_written, target_buffer, buffer_word_length, source.c_str(), source.size() + 1 );
	target = std::wstring( target_buffer );

	delete []target_buffer;
}


void String_To_WideString( const char *source, std::wstring &target )
{
	String_To_WideString( std::string( source ), target );
}


void WideString_To_String( const std::wstring &source, std::string &target )
{
	size_t buffer_length = 2 * ( source.size() + 1 );
	char *target_buffer = new char[ buffer_length ];
	size_t characters_converted = 0;

	wcstombs_s( &characters_converted, target_buffer, buffer_length, source.c_str(), source.size() + 1 );
	target = std::string( target_buffer );

	delete []target_buffer;
}


void WideString_To_String( const wchar_t *source, std::string &target )
{
	WideString_To_String( std::wstring( source ), target );
}


void To_Upper_Case( const std::string &source, std::string &dest )
{
	size_t buffer_size = source.size() + 1;
	char *buffer = new char[ buffer_size ];
	strcpy_s( buffer, buffer_size, source.c_str() );
	_strupr_s( buffer, buffer_size );

	dest = std::string( buffer );

	delete buffer;
}


void To_Upper_Case( const std::wstring &source, std::wstring &dest )
{
	size_t buffer_size = source.size() + 1;
	wchar_t *buffer = new wchar_t[ buffer_size ];
	wcscpy_s( buffer, buffer_size, source.c_str() );
	_wcsupr_s( buffer, buffer_size );

	dest = std::wstring( buffer );

	delete buffer;
}


bool Convert( const std::wstring &source, int32_t &value ) 
{
	return Convert_Raw( source.c_str(), value );
}


bool Convert( const std::wstring &source, uint32_t &value ) 
{
	return Convert_Raw( source.c_str(), value );
}


bool Convert( const std::wstring &source, int64_t &value ) 
{
	return Convert_Raw( source.c_str(), value );
}


bool Convert( const std::wstring &source, uint64_t &value ) 
{
	return Convert_Raw( source.c_str(), value );
}


bool Convert( const std::wstring &source, std::wstring &value ) 
{
	return Convert_Raw( source.c_str(), value );
}


bool Convert( const std::wstring &source, std::string &value ) 
{
	return Convert_Raw( source.c_str(), value );
}


bool Convert( const std::wstring &source, float &value ) 
{
	return Convert_Raw( source.c_str(), value );
}


bool Convert( const std::wstring &source, double &value ) 
{
	return Convert_Raw( source.c_str(), value );
}


bool Convert( const std::wstring &source, bool &value ) 
{
	return Convert_Raw( source.c_str(), value );
}


bool Convert_Raw( const wchar_t *source, int32_t &value ) 
{
	wchar_t *end_ptr = nullptr;
	value = wcstol( source, &end_ptr, 10 );

	return *end_ptr == 0;
}


bool Convert_Raw( const wchar_t *source, uint32_t &value ) 
{
	wchar_t *end_ptr = nullptr;
	value = wcstoul( source, &end_ptr, 10 );
	
	return *end_ptr == 0;
}


bool Convert_Raw( const wchar_t *source, int64_t &value ) 
{
	wchar_t *end_ptr = nullptr;
	value = _wcstoi64( source, &end_ptr, 10 );
	
	return *end_ptr == 0;
}


bool Convert_Raw( const wchar_t *source, uint64_t &value ) 
{
	wchar_t *end_ptr = nullptr;
	value = _wcstoui64( source, &end_ptr, 10 );

	return *end_ptr == 0;
}


bool Convert_Raw( const wchar_t *source, std::wstring &value ) 
{
	value = source;
	return true;
}


bool Convert_Raw( const wchar_t *source, std::string &value ) 
{
	WideString_To_String( source, value );
	return true;
}


bool Convert_Raw( const wchar_t *source, float &value ) 
{
	wchar_t *end_ptr = nullptr;
	double value_d = wcstod( source, &end_ptr );
	value = static_cast< float >( value_d );
	
	return *end_ptr == 0;
}


bool Convert_Raw( const wchar_t *source, double &value ) 
{
	wchar_t *end_ptr = nullptr;
	value = wcstod( source, &end_ptr );
	
	return *end_ptr == 0;
}


bool Convert_Raw( const wchar_t *source, bool &value ) 
{
	value = false;
	if ( _wcsicmp( source, L"TRUE" ) == 0 || _wcsicmp( source, L"YES" ) == 0 || _wcsicmp( source, L"1" ) == 0 )
	{
		value = true;
	}

	return true;
}

} // namespace String
} // namespace IP