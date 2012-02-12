/**********************************************************************************************************************

	StringUtils.cpp
		A component that wraps miscellaneous string functionality.  This logic is not os-specific and thus isn't
		put in the NPlatform namespace.

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

/**********************************************************************************************************************
	NStringUtils::String_To_WideString - converts a regular string to a widestring

		source -- source string
		target -- output parameter to convert to

**********************************************************************************************************************/
void NStringUtils::String_To_WideString( const std::string &source, std::wstring &target )
{
	size_t buffer_length = source.size() * 2 + 2;
	size_t buffer_word_length = buffer_length * sizeof( wchar_t ) / sizeof( uint32 );
	wchar_t *target_buffer = new wchar_t[ buffer_length ];
	size_t bytes_written = 0;

	mbstowcs_s( &bytes_written, target_buffer, buffer_word_length, source.c_str(), source.size() + 1 );
	target = std::wstring( target_buffer );

	delete []target_buffer;
}

/**********************************************************************************************************************
	NStringUtils::String_To_WideString - converts a regular string to a widestring

		source -- source string
		target -- output parameter to convert to

**********************************************************************************************************************/
void NStringUtils::String_To_WideString( const char *source, std::wstring &target )
{
	String_To_WideString( std::string( source ), target );
}

/**********************************************************************************************************************
	NStringUtils::WideString_To_String - converts a wide string to a regular string

		source -- source wide string
		target -- output parameter to store conversion in

**********************************************************************************************************************/
void NStringUtils::WideString_To_String( const std::wstring &source, std::string &target )
{
	size_t buffer_length = 2 * ( source.size() + 1 );
	char *target_buffer = new char[ buffer_length ];
	size_t characters_converted = 0;

	wcstombs_s( &characters_converted, target_buffer, buffer_length, source.c_str(), source.size() + 1 );
	target = std::string( target_buffer );

	delete []target_buffer;
}

/**********************************************************************************************************************
	NStringUtils::WideString_To_String - converts a wide string to a regular string

		source -- source wide string
		target -- output parameter to store conversion in

**********************************************************************************************************************/
void NStringUtils::WideString_To_String( const wchar_t *source, std::string &target )
{
	WideString_To_String( std::wstring( source ), target );
}

/**********************************************************************************************************************
	NStringUtils::To_Upper_Case - converts a string to uppercase characters

		source -- source string
		dest -- output parameter to store conversion in

**********************************************************************************************************************/
void NStringUtils::To_Upper_Case( const std::string &source, std::string &dest )
{
	size_t buffer_size = source.size() + 1;
	char *buffer = new char[ buffer_size ];
	strcpy_s( buffer, buffer_size, source.c_str() );
	_strupr_s( buffer, buffer_size );

	dest = std::string( buffer );

	delete buffer;
}

/**********************************************************************************************************************
	NStringUtils::To_Upper_Case - converts a wide string to uppercase characters

		source -- source wide string
		dest -- output parameter to store conversion in

**********************************************************************************************************************/
void NStringUtils::To_Upper_Case( const std::wstring &source, std::wstring &dest )
{
	size_t buffer_size = source.size() + 1;
	wchar_t *buffer = new wchar_t[ buffer_size ];
	wcscpy_s( buffer, buffer_size, source.c_str() );
	_wcsupr_s( buffer, buffer_size );

	dest = std::wstring( buffer );

	delete buffer;
}

/**********************************************************************************************************************
	NStringUtils::Convert - converts a wide string to a built-in value type

		source -- source wide string
		value -- output parameter to store conversion in

	Returns: conversion success/failure
**********************************************************************************************************************/
bool NStringUtils::Convert( const std::wstring &source, int32 &value ) 
{
	return Convert_Raw( source.c_str(), value );
}

/**********************************************************************************************************************
	NStringUtils::Convert - converts a wide string to a built-in value type

		source -- source wide string
		value -- output parameter to store conversion in

	Returns: conversion success/failure
**********************************************************************************************************************/
bool NStringUtils::Convert( const std::wstring &source, uint32 &value ) 
{
	return Convert_Raw( source.c_str(), value );
}

/**********************************************************************************************************************
	NStringUtils::Convert - converts a wide string to a built-in value type

		source -- source wide string
		value -- output parameter to store conversion in

	Returns: conversion success/failure
**********************************************************************************************************************/
bool NStringUtils::Convert( const std::wstring &source, int64 &value ) 
{
	return Convert_Raw( source.c_str(), value );
}

/**********************************************************************************************************************
	NStringUtils::Convert - converts a wide string to a built-in value type

		source -- source wide string
		value -- output parameter to store conversion in

	Returns: conversion success/failure
**********************************************************************************************************************/
bool NStringUtils::Convert( const std::wstring &source, uint64 &value ) 
{
	return Convert_Raw( source.c_str(), value );
}

/**********************************************************************************************************************
	NStringUtils::Convert - converts a wide string to a built-in value type

		source -- source wide string
		value -- output parameter to store conversion in

	Returns: conversion success/failure
**********************************************************************************************************************/
bool NStringUtils::Convert( const std::wstring &source, std::wstring &value ) 
{
	return Convert_Raw( source.c_str(), value );
}

/**********************************************************************************************************************
	NStringUtils::Convert - converts a wide string to a built-in value type

		source -- source wide string
		value -- output parameter to store conversion in

	Returns: conversion success/failure
**********************************************************************************************************************/
bool NStringUtils::Convert( const std::wstring &source, std::string &value ) 
{
	return Convert_Raw( source.c_str(), value );
}

/**********************************************************************************************************************
	NStringUtils::Convert - converts a wide string to a built-in value type

		source -- source wide string
		value -- output parameter to store conversion in

	Returns: conversion success/failure
**********************************************************************************************************************/
bool NStringUtils::Convert( const std::wstring &source, float &value ) 
{
	return Convert_Raw( source.c_str(), value );
}

/**********************************************************************************************************************
	NStringUtils::Convert - converts a wide string to a built-in value type

		source -- source wide string
		value -- output parameter to store conversion in

	Returns: conversion success/failure
**********************************************************************************************************************/
bool NStringUtils::Convert( const std::wstring &source, double &value ) 
{
	return Convert_Raw( source.c_str(), value );
}

/**********************************************************************************************************************
	NStringUtils::Convert - converts a wide string to a built-in value type

		source -- source wide string
		value -- output parameter to store conversion in

	Returns: conversion success/failure
**********************************************************************************************************************/
bool NStringUtils::Convert( const std::wstring &source, bool &value ) 
{
	return Convert_Raw( source.c_str(), value );
}

/**********************************************************************************************************************
	NStringUtils::Convert_Raw - converts a wide string to a built-in value type

		source -- source wide string
		value -- output parameter to store conversion in

	Returns: conversion success/failure
**********************************************************************************************************************/
bool NStringUtils::Convert_Raw( const wchar_t *source, int32 &value ) 
{
	wchar_t *end_ptr = nullptr;
	value = wcstol( source, &end_ptr, 10 );

	return *end_ptr == 0;
}

/**********************************************************************************************************************
	NStringUtils::Convert_Raw - converts a wide string to a built-in value type

		source -- source wide string
		value -- output parameter to store conversion in

	Returns: conversion success/failure
**********************************************************************************************************************/
bool NStringUtils::Convert_Raw( const wchar_t *source, uint32 &value ) 
{
	wchar_t *end_ptr = nullptr;
	value = wcstoul( source, &end_ptr, 10 );
	
	return *end_ptr == 0;
}

/**********************************************************************************************************************
	NStringUtils::Convert_Raw - converts a wide string to a built-in value type

		source -- source wide string
		value -- output parameter to store conversion in

	Returns: conversion success/failure
**********************************************************************************************************************/
bool NStringUtils::Convert_Raw( const wchar_t *source, int64 &value ) 
{
	wchar_t *end_ptr = nullptr;
	value = _wcstoi64( source, &end_ptr, 10 );
	
	return *end_ptr == 0;
}

/**********************************************************************************************************************
	NStringUtils::Convert_Raw - converts a wide string to a built-in value type

		source -- source wide string
		value -- output parameter to store conversion in

	Returns: conversion success/failure
**********************************************************************************************************************/
bool NStringUtils::Convert_Raw( const wchar_t *source, uint64 &value ) 
{
	wchar_t *end_ptr = nullptr;
	value = _wcstoui64( source, &end_ptr, 10 );

	return *end_ptr == 0;
}

/**********************************************************************************************************************
	NStringUtils::Convert_Raw - converts a wide string to a built-in value type

		source -- source wide string
		value -- output parameter to store conversion in

	Returns: conversion success/failure
**********************************************************************************************************************/
bool NStringUtils::Convert_Raw( const wchar_t *source, std::wstring &value ) 
{
	value = source;
	return true;
}

/**********************************************************************************************************************
	NStringUtils::Convert_Raw - converts a wide string to a built-in value type

		source -- source wide string
		value -- output parameter to store conversion in

	Returns: conversion success/failure
**********************************************************************************************************************/
bool NStringUtils::Convert_Raw( const wchar_t *source, std::string &value ) 
{
	WideString_To_String( source, value );
	return true;
}

/**********************************************************************************************************************
	NStringUtils::Convert_Raw - converts a wide string to a built-in value type

		source -- source wide string
		value -- output parameter to store conversion in

	Returns: conversion success/failure
**********************************************************************************************************************/
bool NStringUtils::Convert_Raw( const wchar_t *source, float &value ) 
{
	wchar_t *end_ptr = nullptr;
	double value_d = wcstod( source, &end_ptr );
	value = static_cast< float >( value_d );
	
	return *end_ptr == 0;
}

/**********************************************************************************************************************
	NStringUtils::Convert_Raw - converts a wide string to a built-in value type

		source -- source wide string
		value -- output parameter to store conversion in

	Returns: conversion success/failure
**********************************************************************************************************************/
bool NStringUtils::Convert_Raw( const wchar_t *source, double &value ) 
{
	wchar_t *end_ptr = nullptr;
	value = wcstod( source, &end_ptr );
	
	return *end_ptr == 0;
}

/**********************************************************************************************************************
	NStringUtils::Convert_Raw - converts a wide string to a built-in value type

		source -- source wide string
		value -- output parameter to store conversion in

	Returns: conversion success/failure
**********************************************************************************************************************/
bool NStringUtils::Convert_Raw( const wchar_t *source, bool &value ) 
{
	value = false;
	if ( _wcsicmp( source, L"TRUE" ) == 0 || _wcsicmp( source, L"YES" ) == 0 || _wcsicmp( source, L"1" ) == 0 )
	{
		value = true;
	}

	return true;
}
