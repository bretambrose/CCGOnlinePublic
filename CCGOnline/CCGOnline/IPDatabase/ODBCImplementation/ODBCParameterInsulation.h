/**********************************************************************************************************************

	(c) Copyright 2012, Bret Ambrose (mailto:bretambrose@gmail.com).

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

#define INSULATE_FROM_WINDOWS_HEADERS

#ifdef INSULATE_FROM_WINDOWS_HEADERS

/*
using SQLSMALLINT = short;

using SQLRETURN = SQLSMALLINT;

using SQLHANDLE = void*;

using SQLHENV = SQLHANDLE;
using SQLHDBC = SQLHANDLE;
using SQLHSTMT = SQLHANDLE;
using SQLHDESC = SQLHANDLE;

*/

static const int32_t IP_SQL_NULL_DATA = -1;
static const int32_t IP_SQL_NTS = -3;

#ifdef X64
#define IP_SQLLEN long long
#else
#define IP_SQLLEN long
#endif

#else

#include "IPPlatform/WindowsWrapper.h"
#include <sql.h>
#include <sqlext.h>
#include <sqltypes.h>

static const int32_t IP_SQL_NULL_DATA = SQL_NULL_DATA;
#define IP_SQLLEN SQLLEN
#define IP_SQL_NTS SQL_NTS

#endif // INSULATE_FROM_WINDOWS_HEADERS

