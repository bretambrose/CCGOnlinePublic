/**********************************************************************************************************************

	DatabaseParameterSetInterface.h
		A component defining the abstract interface to a stored procedure's input parameters.

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

#ifndef DATABASE_PARAMETER_SET_INTERFACE_H
#define DATABASE_PARAMETER_SET_INTERFACE_H

class IDatabaseParameter;

class IDatabaseParameterSet
{
	public:

		virtual ~IDatabaseParameterSet() {}

		virtual void Get_Parameters( std::vector< IDatabaseParameter * > &params ) = 0;
};

#endif // DATABASE_PARAMETER_SET_INTERFACE_H