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

enum DBErrorStateType;
enum EFetchResultsStatusType;
enum DBStatementIDType;

namespace IP
{
namespace Db
{

class IDatabaseVariableSet;
class IDatabaseConnection;

class IDatabaseStatement
{
	public:

		virtual ~IDatabaseStatement() {}

		virtual void Initialize( const std::wstring &statement_text ) = 0;
		virtual void Shutdown( void ) = 0;

		virtual DBStatementIDType Get_ID( void ) const = 0;
		virtual const std::wstring &Get_Statement_Text( void ) const = 0;

		virtual void Bind_Input( IDatabaseVariableSet *param_set, uint32_t param_set_size ) = 0;
		virtual void Bind_Output( IDatabaseVariableSet *result_set, uint32_t result_set_size, uint32_t result_set_count ) = 0;
		virtual void Execute( uint32_t batch_size ) = 0;
		virtual EFetchResultsStatusType Fetch_Results( int64_t &rows_fetched ) = 0;
		virtual void Return_To_Ready( void ) = 0;

		virtual bool Needs_Binding( void ) const = 0;
		virtual bool Is_Ready_For_Use( void ) const = 0;
		virtual bool Is_In_Error_State( void ) const = 0;
		virtual bool Should_Have_Results( void ) const = 0;
		virtual IDatabaseConnection *Get_Connection( void ) const = 0;

		virtual DBErrorStateType Get_Error_State( void ) const = 0;
		virtual int32_t Get_Bad_Row_Number( void ) const = 0;
		virtual void Log_Error_State( void ) const = 0;
};

} // namespace Db
} // namespace IP