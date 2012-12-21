/**********************************************************************************************************************

	ODBCStatement.h
		A component defining an ODBC implementation of the abstract database statement interface.

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

#ifndef ODBC_STATEMENT_H
#define ODBC_STATEMENT_H

#include "Database/Interfaces/DatabaseStatementInterface.h"
#include "ODBCObjectBase.h"

enum ODBCStatementStateType;
enum ODBCStatementOperationType;

class CODBCStatement : public CODBCObjectBase, public IDatabaseStatement
{
	public:

		typedef CODBCObjectBase BASECLASS;

		CODBCStatement( DBStatementIDType id, SQLHENV environment_handle, SQLHDBC connection_handle, SQLHSTMT statement_handle );
		virtual ~CODBCStatement();

		virtual void Initialize( const std::wstring &statement_text );
		virtual void Shutdown( void );

		virtual DBStatementIDType Get_ID( void ) const { return ID; }
		virtual const std::wstring &Get_Statement_Text( void ) const { return StatementText; }

		virtual void Bind_Input( IDatabaseVariableSet *param_set, uint32 param_set_size );
		virtual void Bind_Output( IDatabaseVariableSet *result_set, uint32 result_set_size, uint32 result_set_count );
		virtual void Execute( uint32 batch_size );
		virtual void End_Transaction( bool commit );
		virtual EFetchResultsStatusType Fetch_Results( int64 &rows_fetched );

		virtual bool Needs_Binding( void ) const;
		virtual bool Is_Ready_For_Use( void ) const;

		virtual DBErrorStateType Get_Error_State( void ) const { return Get_Error_State_Base(); }
		virtual int32 Get_Bad_Row_Number( void ) const { return Get_Bad_Row_Number_Base(); }

	private:

		bool Was_Last_ODBC_Operation_Successful( void ) const;

		void Update_Error_Status( ODBCStatementOperationType operation_type, SQLRETURN error_code );
		
		DBStatementIDType ID;

		ODBCStatementStateType State;

		std::wstring StatementText;

		uint32 ResultSetRowCount;

		SQLSMALLINT *RowStatuses;
		int64 RowsFetched;

		bool ProcessResults;
};

#endif // ODBC_STATEMENT_H