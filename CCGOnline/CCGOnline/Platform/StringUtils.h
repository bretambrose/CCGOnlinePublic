/**********************************************************************************************************************

	StringUtils.h
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