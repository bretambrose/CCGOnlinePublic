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

#include <IPDatabase/Interfaces/DatabaseStatementInterface.h>
#include <IPDatabase/ODBCImplementation/ODBCObjectBase.h>

namespace IP
{
namespace Db
{

enum class ODBCStatementStateType;
enum class ODBCStatementOperationType;

IPDATABASE_API class CODBCStatement : public CODBCObjectBase, public IDatabaseStatement
{
	public:

		using BASECLASS = CODBCObjectBase;

		CODBCStatement( DBStatementIDType id, IDatabaseConnection *connection, SQLHENV environment_handle, SQLHDBC connection_handle, SQLHSTMT statement_handle );
		virtual ~CODBCStatement();

		virtual void Initialize( const IP::String &statement_text );
		virtual void Shutdown( void );

		virtual DBStatementIDType Get_ID( void ) const { return ID; }
		virtual const IP::String &Get_Statement_Text( void ) const { return StatementText; }

		virtual void Bind_Input( IDatabaseVariableSet *param_set, uint32_t param_set_size );
		virtual void Bind_Output( IDatabaseVariableSet *result_set, uint32_t result_set_size, uint32_t result_set_count );
		virtual void Execute( uint32_t batch_size );
		virtual EFetchResultsStatusType Fetch_Results( int64_t &rows_fetched );
		virtual void Return_To_Ready( void );

		virtual bool Needs_Binding( void ) const;
		virtual bool Is_Ready_For_Use( void ) const;
		virtual bool Is_In_Error_State( void ) const;
		virtual bool Should_Have_Results( void ) const { return ExpectedResultSetWidth > 0; }
		virtual IDatabaseConnection *Get_Connection( void ) const { return Connection; }

		virtual DBErrorStateType Get_Error_State( void ) const { return Get_Error_State_Base(); }
		virtual int32_t Get_Bad_Row_Number( void ) const { return Get_Bad_Row_Number_Base(); }
		virtual void Log_Error_State( void ) const;

	private:

		bool Was_Last_ODBC_Operation_Successful( void ) const;

		void Update_Error_Status( ODBCStatementOperationType operation_type, SQLRETURN error_code );
		void Reflect_ODBC_Error_State_Into_Statement_State( void );

		DBStatementIDType ID;

		IDatabaseConnection *Connection;

		ODBCStatementStateType State;

		IP::String StatementText;

		uint32_t ResultSetRowCount;

		SQLSMALLINT *RowStatuses;
		int64_t RowsFetched;

		int32_t ExpectedResultSetWidth;
		int32_t CurrentResultSetWidth;
		int32_t CurrentResultSet;

};

} // namespace Db
} // namespace IP