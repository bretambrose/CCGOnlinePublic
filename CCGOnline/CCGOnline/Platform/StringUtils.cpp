/**********************************************************************************************************************

	[Placeholder for eventual source license]

	StringUtils.cpp
		A component that wraps miscellaneous string functionality.  This logic is not os-specific and thus isn't
		put in the NPlatform namespace.

	(c) Copyright 2011, Bret Ambrose.  All rights reserved.

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

