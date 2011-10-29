/**********************************************************************************************************************

	[Placeholder for eventual source license]

	StringUtils.h
		A component that wraps miscellaneous string functionality.  This logic is not os-specific and thus isn't
		put in the NPlatform namespace.

	(c) Copyright 2011, Bret Ambrose.  All rights reserved.

**********************************************************************************************************************/

#ifndef STRING_UTILS_H
#define STRING_UTILS_H

namespace NStringUtils
{
	void String_To_WideString( const std::string &source, std::wstring &target );
	void String_To_WideString( const char *source, std::wstring &target );

	void WideString_To_String( const std::wstring &source, std::string &target );
	void WideString_To_String( const wchar_t *source, std::string &target );

	void To_Upper_Case( const std::string &source, std::string &dest );
	void To_Upper_Case( const std::wstring &source, std::wstring &dest );
}

#endif // STRING_UTILS_H