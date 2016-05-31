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

#include <IPCore/Memory/Stl/String.h>


namespace IP
{
namespace StringUtils
{

	void String_To_WideString( const IP::String &source, IP::WString &target );
	void String_To_WideString( const char *source, IP::WString &target );

	void WideString_To_String( const IP::WString &source, IP::String &target );
	void WideString_To_String( const wchar_t *source, IP::String &target );

	void To_Upper_Case( const IP::String &source, IP::String &dest );
	void To_Upper_Case( const IP::WString &source, IP::WString &dest );

	bool Convert( const IP::String &source, int32_t &value );
	bool Convert( const IP::String &source, uint32_t &value );
	bool Convert( const IP::String &source, int64_t &value );
	bool Convert( const IP::String &source, uint64_t &value );
	bool Convert( const IP::String &source, IP::WString &value );
	bool Convert( const IP::String &source, IP::String &value );
	bool Convert( const IP::String &source, float &value );
	bool Convert( const IP::String &source, double &value );
	bool Convert( const IP::String &source, bool &value );

	bool Convert_Raw( const char *source, int32_t &value );
	bool Convert_Raw( const char *source, uint32_t &value );
	bool Convert_Raw( const char *source, int64_t &value );
	bool Convert_Raw( const char *source, uint64_t &value );
	bool Convert_Raw( const char *source, IP::WString &value );
	bool Convert_Raw( const char *source, IP::String &value );
	bool Convert_Raw( const char *source, float &value );
	bool Convert_Raw( const char *source, double &value );
	bool Convert_Raw( const char *source, bool &value );

} // namespace StringUtils
} // namespace IP

