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
namespace String
{

	void String_To_WideString( const std::string &source, std::wstring &target );
	void String_To_WideString( const char *source, std::wstring &target );

	void WideString_To_String( const std::wstring &source, std::string &target );
	void WideString_To_String( const wchar_t *source, std::string &target );

	void To_Upper_Case( const std::string &source, std::string &dest );
	void To_Upper_Case( const std::wstring &source, std::wstring &dest );

	bool Convert( const std::wstring &source, int32_t &value );
	bool Convert( const std::wstring &source, uint32_t &value );
	bool Convert( const std::wstring &source, int64_t &value );
	bool Convert( const std::wstring &source, uint64_t &value );
	bool Convert( const std::wstring &source, std::wstring &value );
	bool Convert( const std::wstring &source, std::string &value );
	bool Convert( const std::wstring &source, float &value );
	bool Convert( const std::wstring &source, double &value );
	bool Convert( const std::wstring &source, bool &value );

	bool Convert_Raw( const wchar_t *source, int32_t &value );
	bool Convert_Raw( const wchar_t *source, uint32_t &value );
	bool Convert_Raw( const wchar_t *source, int64_t &value );
	bool Convert_Raw( const wchar_t *source, uint64_t &value );
	bool Convert_Raw( const wchar_t *source, std::wstring &value );
	bool Convert_Raw( const wchar_t *source, std::string &value );
	bool Convert_Raw( const wchar_t *source, float &value );
	bool Convert_Raw( const wchar_t *source, double &value );
	bool Convert_Raw( const wchar_t *source, bool &value );

} // namespace String
} // namespace IP

#undef _STRING_
#undef RC_INVOKED

#define basic_string_test test_string

template< typename T >
class basic_string_test
{
	public:

		basic_string_test() : blah(0) {}
		basic_string_test(const basic_string_test& rhs ) : blah( rhs.blah ) {}

	private:

		uint32_t blah;
};

#undef basic_string

namespace Aws
{
using String = test_string<char>;
}