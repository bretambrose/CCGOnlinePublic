/**********************************************************************************************************************

	DatabaseConnectionInterface.h
		A component defining the abstract interface to a database connection.  This is a wrapper of the ODBC connection handle.

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

#ifndef DATABASE_CONNECTION_INTERFACE_H
#define DATABASE_CONNECTION_INTERFACE_H

class IDatabaseStatement;
class IDatabaseTask;
class IDatabaseVariableSet;

enum DBConnectionIDType;
enum DBErrorStateType;
enum EDatabaseTaskType;

class IDatabaseConnection
{
	public:

		virtual ~IDatabaseConnection() {}

		virtual void Initialize( const std::wstring &connection_string ) = 0;
		virtual void Shutdown( void ) = 0;

		virtual DBConnectionIDType Get_ID( void ) const = 0;

		virtual IDatabaseStatement *Allocate_Statement( const std::wstring &statement_text ) = 0;
		virtual void Release_Statement( IDatabaseStatement *statement ) = 0;

		virtual DBErrorStateType Get_Error_State( void ) const = 0;

		virtual void Construct_Statement_Text( IDatabaseTask *task, IDatabaseVariableSet *input_parameters, std::wstring &statement_text ) const = 0;
		virtual bool Validate_Input_Output_Signatures( EDatabaseTaskType task_type, IDatabaseVariableSet *input_parameters, IDatabaseVariableSet *output_parameters ) const = 0;

};

#endif // DATABASE_CONNECTION_INTERFACE_H