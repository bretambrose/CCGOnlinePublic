/**********************************************************************************************************************

	EmptyVariableSet.h
		A component defining some base instances of a set of application variables.

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

#ifndef EMPTY_VARIABLE_SET_H
#define EMPTY_VARIABLE_SET_H

#include "Database/Interfaces/DatabaseVariableSetInterface.h"

class CEmptyVariableSet : public IDatabaseVariableSet
{
	public:

		CEmptyVariableSet( void )
		{}

		virtual ~CEmptyVariableSet() {}

		virtual void Get_Variables( std::vector< IDatabaseVariable * > & /*variables*/ )
		{
		}

		virtual void Convert_Variable_To_String( IDatabaseVariable * /*variable*/, std::string & /*value*/ ) const
		{
		}

};

#endif // EMPTY_VARIABLE_SET_H