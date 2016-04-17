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

#include <IPDatabase/IPDatabase.h>

#include <IPCore/Memory/Stl/UnorderedMap.h>
#include <IPDatabase/Interfaces/DatabaseConnectionInterface.h>
#include <IPDatabase/ODBCImplementation/ODBCObjectBase.h>

enum DBStatementIDType;

namespace IP
{
namespace Db
{

enum class ODBCConnectionStateType;
enum class ODBCConnectionOperationType;

IPDATABASE_API class CODBCConnection : public CODBCObjectBase, public IDatabaseConnection
{
	public:

		CODBCConnection( DBConnectionIDType id, SQLHENV environment_handle, SQLHDBC connection_handle, bool cache_statements );
		virtual ~CODBCConnection();

		virtual void Initialize( const IP::String &connection_string );
		virtual void Shutdown( void );

		virtual DBConnectionIDType Get_ID( void ) const { return ID; }

		virtual IDatabaseStatement *Allocate_Statement( const IP::String &statement_text );
		virtual void Release_Statement( IDatabaseStatement *statement );
		virtual void End_Transaction( bool commit );

		virtual DBErrorStateType Get_Error_State( void ) const { return Get_Error_State_Base(); }

		virtual void Construct_Statement_Text( IDatabaseTask *task, IDatabaseVariableSet *input_parameters, IP::String &statement_text ) const;
		virtual bool Validate_Input_Output_Signatures( IDatabaseTask *task, IDatabaseVariableSet *input_parameters, IDatabaseVariableSet *output_parameters ) const;

	private:

		void Construct_Function_Procedure_Call_Statement_Text( IDatabaseTask *task, IDatabaseVariableSet *input_parameters, IP::String &statement_text ) const;
		void Construct_Select_Statement_Text( IDatabaseTask *task, IP::String &statement_text ) const;
		void Construct_Table_Valued_Function_Statement_Text( IDatabaseTask *task, IDatabaseVariableSet *input_parameters, IP::String &statement_text ) const;

		IDatabaseStatement *Get_Statement( DBStatementIDType id ) const;
		
		bool Was_Last_ODBC_Operation_Successful( void ) const;

		void Update_Error_Status( ODBCConnectionOperationType operation_type, SQLRETURN error_code );
				
		DBConnectionIDType ID;

		ODBCConnectionStateType State;

		IP::UnorderedMap< DBStatementIDType, IDatabaseStatement * > Statements;
		IP::UnorderedMap< IP::String, DBStatementIDType > CachedStatements;

		DBStatementIDType NextStatementID;

		bool UseStatementCaching;
};

} // namespace Db
} // namespace IP