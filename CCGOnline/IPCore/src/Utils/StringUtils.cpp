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

void String_To_WideString( const IP::String &source, IP::WString &target )
{
	std::mbstate_t state = std::mbstate_t();
	auto source_str = source.c_str();
	size_t len = 1 + std::mbsrtowcs(NULL, &source_str, 0, &state);

	target.clear();
	target.resize( len );

	std::mbsrtowcs(&target[0], &source_str, len, &state);
}


void WideString_To_String( const IP::WString &source, IP::String &target )
{
	std::mbstate_t state = std::mbstate_t();
	auto source_str = source.c_str();
	size_t len = 1 + std::wcsrtombs(nullptr, &source_str, 0, &state);

	target.clear();
	target.resize( len );

	std::wcsrtombs(&target[0], &source_str, len, &state);
}


void String_To_WideString( const char *source, IP::WString &target )
{
	String_To_WideString( IP::String( source ), target );
}


void WideString_To_String( const wchar_t *source, IP::String &target )
{
	WideString_To_String( IP::WString( source ), target );
}


void To_Upper_Case( const IP::String &source, IP::String &dest )
{
	dest.clear();
	dest.resize( source.size() );

	for(size_t i = 0, end = source.size(); i < end; ++i)
	{
		dest[ i ] = std::toupper( source[ i ] );
	}
}


void To_Upper_Case( const IP::WString &source, IP::WString &dest )
{
	dest.clear();
	dest.resize( source.size() );

	for(size_t i = 0, end = source.size(); i < end; ++i)
	{
		dest[ i ] = std::toupper( source[ i ] );
	}
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


} // namespace String
} // namespace IP