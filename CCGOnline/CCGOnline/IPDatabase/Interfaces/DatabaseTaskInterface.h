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

#include "DatabaseTaskBaseInterface.h"

#include "IPDatabase/DatabaseTypes.h"
#include "IPDatabase/DatabaseTaskBatchUtilities.h"

enum EDatabaseTaskType;

namespace IP
{
namespace Db
{

class IDatabaseVariableSet;
class IDatabaseCallContext;
class IDatabaseStatement;

enum class ExecuteDBTaskListResult;

class IDatabaseTask : public IDatabaseTaskBase
{
	public:
		
		using BASECLASS = IDatabaseTaskBase;

		IDatabaseTask( void ) :
			BASECLASS()
		{}

		virtual ~IDatabaseTask() {}

		virtual const wchar_t *Get_Database_Object_Name( void ) const = 0;
		virtual void Build_Column_Name_List( std::vector< const wchar_t * > &column_names ) const = 0;
		virtual EDatabaseTaskType Get_Task_Type( void ) const = 0;	

	protected:

		template < typename T > friend class TDatabaseTaskBatch;
		friend void Execute_Task_List( IDatabaseCallContext *, IDatabaseStatement *, const DBTaskListType &, EExecuteDBTaskListResult &, DBTaskListType::const_iterator & );

		virtual void Initialize_Parameters( IDatabaseVariableSet *input_parameters ) = 0;		
		virtual void On_Fetch_Results( IDatabaseVariableSet *result_set, int64_t rows_fetched ) = 0;			
		virtual void On_Fetch_Results_Finished( IDatabaseVariableSet *input_parameters ) = 0;	

		virtual void On_Rollback( void ) = 0;			

};

} // namespace Db
} // namespace IP