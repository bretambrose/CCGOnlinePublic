/**********************************************************************************************************************

	DatabaseResultColumnInterface.h
		A component defining the abstract interface to an individual result column of a stored procedure.

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

#ifndef DATABASE_RESULT_COLUMN_INTERFACE_H
#define DATABASE_RESULT_COLUMN_INTERFACE_H

enum EDatabaseParameterValueType;

class IDatabaseResultColumn
{
	public:

		virtual ~IDatabaseResultColumn() {}

		virtual EDatabaseParameterValueType Get_Value_Type( void ) const = 0;
		virtual void *Get_Value_Address( void ) const = 0;
		virtual uint32 Get_Value_Buffer_Size( void ) const = 0;
		virtual void *Get_Auxiliary_Address( void ) const = 0;
};

#endif // DATABASE_RESULT_COLUMN_INTERFACE_H