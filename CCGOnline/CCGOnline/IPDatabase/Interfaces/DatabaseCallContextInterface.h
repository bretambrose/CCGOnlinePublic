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

#ifndef DATABASE_CALL_CONTEXT_INTERFACE_H
#define DATABASE_CALL_CONTEXT_INTERFACE_H

class IDatabaseVariableSet;

class IDatabaseCallContext 
{
	public:

		IDatabaseCallContext() {}
		virtual ~IDatabaseCallContext() {}

		virtual IDatabaseVariableSet *Get_Param_Rows( void ) = 0;
		virtual IDatabaseVariableSet *Get_Param_Row( uint32_t index ) = 0;
		virtual uint32_t Get_Param_Row_Count( void ) const = 0;
		virtual uint32_t Get_Sizeof_Param_Type( void ) const = 0;

		virtual IDatabaseVariableSet *Get_Result_Rows( void ) = 0;
		virtual IDatabaseVariableSet *Get_Result_Row( uint32_t index ) = 0;
		virtual uint32_t Get_Result_Row_Count( void ) const = 0;
		virtual uint32_t Get_Sizeof_Result_Type( void ) const = 0;

		virtual const std::wstring & Get_Statement_Text( void ) const = 0;
		virtual void Set_Statement_Text( const std::wstring &statement_text ) = 0;
};

#endif // DATABASE_CALL_CONTEXT_INTERFACE_H