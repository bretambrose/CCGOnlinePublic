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

#include <IPCore/Utils/StringUtils.h>

#include <IPCore/Memory/Stl/StringStream.h>

namespace IP
{
namespace StringUtils
{

static const char *AllocationTag = "StringUtils";

void String_To_WideString( const IP::String &source, IP::WString &target )
{
	size_t buffer_length = source.size() * 2 + 2;
	size_t buffer_word_length = buffer_length * sizeof( wchar_t ) / sizeof( uint32_t );
	wchar_t *target_buffer = IP::New_Array< wchar_t >( AllocationTag, buffer_length );
	size_t bytes_written = 0;

	mbstowcs_s( &bytes_written, target_buffer, buffer_word_length, source.c_str(), source.size() + 1 );
	target = IP::WString( target_buffer );

	IP::Delete_Array( target_buffer );
}


void String_To_WideString( const char *source, IP::WString &target )
{
	String_To_WideString( IP::String( source ), target );
}


void WideString_To_String( const IP::WString &source, IP::String &target )
{
	size_t buffer_length = 2 * ( source.size() + 1 );
	char *target_buffer = IP::New_Array< char >( AllocationTag, buffer_length );
	size_t characters_converted = 0;

	wcstombs_s( &characters_converted, target_buffer, buffer_length, source.c_str(), source.size() + 1 );
	target = IP::String( target_buffer );

	IP::Delete_Array( target_buffer );
}


void WideString_To_String( const wchar_t *source, IP::String &target )
{
	WideString_To_String( IP::WString( source ), target );
}


void To_Upper_Case( const IP::String &source, IP::String &dest )
{
	size_t buffer_size = source.size() + 1;
	char *buffer = IP::New_Array< char >( AllocationTag, buffer_size );
	strcpy_s( buffer, buffer_size, source.c_str() );
	_strupr_s( buffer, buffer_size );

	dest = IP::String( buffer );

	IP::Delete_Array( buffer );
}


void To_Upper_Case( const IP::WString &source, IP::WString &dest )
{
	size_t buffer_size = source.size() + 1;
	wchar_t *buffer = IP::New_Array< wchar_t >( AllocationTag, buffer_size );
	wcscpy_s( buffer, buffer_size, source.c_str() );
	_wcsupr_s( buffer, buffer_size );

	dest = IP::WString( buffer );

	IP::Delete_Array( buffer );
}


bool Convert( const IP::String &source, int32_t &value ) 
{
	return Convert_Raw( source.c_str(), value );
}


bool Convert( const IP::String &source, uint32_t &value ) 
{
	return Convert_Raw( source.c_str(), value );
}


bool Convert( const IP::String &source, int64_t &value ) 
{
	return Convert_Raw( source.c_str(), value );
}


bool Convert( const IP::String &source, uint64_t &value ) 
{
	return Convert_Raw( source.c_str(), value );
}


bool Convert( const IP::String &source, IP::String &value ) 
{
	return Convert_Raw( source.c_str(), value );
}


bool Convert( const IP::String &source, IP::WString &value ) 
{
	return Convert_Raw( source.c_str(), value );
}


bool Convert( const IP::String &source, float &value ) 
{
	return Convert_Raw( source.c_str(), value );
}


bool Convert( const IP::String &source, double &value ) 
{
	return Convert_Raw( source.c_str(), value );
}


bool Convert( const IP::String &source, bool &value ) 
{
	return Convert_Raw( source.c_str(), value );
}


bool Convert_Raw( const char *source, int32_t &value ) 
{
	char *end_ptr = nullptr;
	value = strtol( source, &end_ptr, 10 );

	return *end_ptr == 0;
}


bool Convert_Raw( const char *source, uint32_t &value ) 
{
	char *end_ptr = nullptr;
	value = strtoul( source, &end_ptr, 10 );
	
	return *end_ptr == 0;
}


bool Convert_Raw( const char *source, int64_t &value ) 
{
	IP::StringStream ss;
    
	ss << source;
	ss >> value;

	return true;
}


bool Convert_Raw( const char *source, uint64_t &value ) 
{
	IP::StringStream ss;
    
	ss << source;
	ss >> value;

	return true;
}


bool Convert_Raw( const char *source, IP::WString &value ) 
{
	String_To_WideString( source, value );
	return true;
}


bool Convert_Raw( const char *source, IP::String &value ) 
{
	value = source;
	return true;
}


bool Convert_Raw( const char *source, float &value ) 
{
	char *end_ptr = nullptr;
	double value_d = strtod( source, &end_ptr );
	value = static_cast< float >( value_d );
	
	return *end_ptr == 0;
}


bool Convert_Raw( const char *source, double &value ) 
{
	char *end_ptr = nullptr;
	value = strtod( source, &end_ptr );
	
	return *end_ptr == 0;
}


bool Convert_Raw( const char *source, bool &value ) 
{
	value = false;
	if ( _stricmp( source, "TRUE" ) == 0 || _stricmp( source, "YES" ) == 0 || _stricmp( source, "1" ) == 0 )
	{
		value = true;
	}

	return true;
}

} // namespace String
} // namespace IP