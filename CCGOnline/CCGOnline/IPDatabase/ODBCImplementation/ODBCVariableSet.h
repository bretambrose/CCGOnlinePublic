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

#ifndef ODBC_VARIABLE_SET_H
#define ODBC_VARIABLE_SET_H

#include "IPDatabase/Interfaces/DatabaseVariableSetInterface.h"

class CODBCVariableSet : public IDatabaseVariableSet
{
	public:

		typedef IDatabaseVariableSet BASECLASS;

		CODBCVariableSet( void ) {}
		CODBCVariableSet( const CODBCVariableSet & /*rhs*/ ) {}

		virtual void Convert_Variable_To_String( IDatabaseVariable *variable, std::string &value ) const;
};

#endif // ODBC_VARIABLE_SET_H
