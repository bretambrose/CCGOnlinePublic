/**********************************************************************************************************************

	ODBCParameters.cpp
		A component defining various ODBC specific parameter types to be used as inputs to stored procedures

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

#include "stdafx.h"

#include "ODBCParameters.h"

#include <sstream>

// Split off in a global function to avoid the stream dependence in the parameter header file
void Convert_Database_Variable_To_String( IDatabaseVariable *variable, std::string &value )
{
	std::basic_ostringstream< char > varstream;

	switch ( variable->Get_Value_Type() )
	{
		case DVVT_INT32:
		{
			TODBCScalarVariableBase< int32 > *scalar_variable = static_cast< TODBCScalarVariableBase< int32 > * >( variable );
			varstream << scalar_variable->Get_Value();
			break;
		}

		case DVVT_UINT32:
		{
			TODBCScalarVariableBase< uint32 > *scalar_variable = static_cast< TODBCScalarVariableBase< uint32 > * >( variable );
			varstream << scalar_variable->Get_Value();
			break;
		}

		case DVVT_INT64:
		{
			TODBCScalarVariableBase< int64 > *scalar_variable = static_cast< TODBCScalarVariableBase< int64 > * >( variable );
			varstream << scalar_variable->Get_Value();
			break;
		}

		case DVVT_UINT64:
		{
			TODBCScalarVariableBase< uint64 > *scalar_variable = static_cast< TODBCScalarVariableBase< uint64 > * >( variable );
			varstream << scalar_variable->Get_Value();
			break;
		}

		case DVVT_STRING:
		case DVVT_WSTRING:
		{
			IDatabaseString *db_string = static_cast< IDatabaseString * >( variable );
			db_string->To_String( value );
			return;
		}

		case DVVT_FLOAT:
		{
			TODBCScalarVariableBase< float > *scalar_variable = static_cast< TODBCScalarVariableBase< float > * >( variable );
			varstream << scalar_variable->Get_Value();
			break;
		}

		case DVVT_DOUBLE:
		{
			TODBCScalarVariableBase< double > *scalar_variable = static_cast< TODBCScalarVariableBase< double > * >( variable );
			varstream << scalar_variable->Get_Value();
			break;
		}

		case DVVT_BOOLEAN:
		{
			TODBCScalarVariableBase< bool > *scalar_variable = static_cast< TODBCScalarVariableBase< bool > * >( variable );
			varstream << ( scalar_variable->Get_Value() ? "true" : "false" );
			break;
		}
	}

	value = varstream.rdbuf()->str();
}
