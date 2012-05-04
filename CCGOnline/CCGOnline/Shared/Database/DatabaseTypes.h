/**********************************************************************************************************************

	DatabaseTypes.h
		A component defining some enums, typedefs, and constants for various database object IDs

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

#ifndef DATABASE_TYPES_H
#define DATABASE_TYPES_H

enum DBErrorStateType
{
	DBEST_SUCCESS,

	DBEST_FATAL_ERROR,
	DBEST_NON_FATAL_ERROR,
	DBEST_WARNING
};

inline bool Is_Fatal_DB_Error( DBErrorStateType error ) { return error == DBEST_FATAL_ERROR; }
inline bool Was_Database_Operation_Successful( DBErrorStateType error ) { return error == DBEST_SUCCESS || error == DBEST_WARNING; }

enum DBConnectionIDType
{
	DBCIDT_INVALID
};

enum DBStatementIDType
{
	DBSIDT_INVALID
};

enum EDatabaseVariableType
{
	DVT_INPUT,
	DVT_INPUT_OUTPUT,
	DVT_OUTPUT
};

enum EDatabaseVariableValueType
{
	DVVT_INVALID,

	DVVT_INT32,
	DVVT_UINT32,
	DVVT_INT64,
	DVVT_UINT64,
	DVVT_STRING,
	DVVT_WSTRING,

	DVVT_FLOAT,
	DVVT_DOUBLE,
	DVVT_BOOLEAN
};

#endif // DATABASE_TYPES_H