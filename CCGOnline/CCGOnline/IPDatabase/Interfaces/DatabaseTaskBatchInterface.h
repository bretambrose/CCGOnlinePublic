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

#ifndef DATABASE_TASK_BATCH_INTERFACE_H
#define DATABASE_TASK_BATCH_INTERFACE_H

#include "IPDatabase/DatabaseTypes.h"

class ICompoundDatabaseTask;
class IDatabaseTask;
class IDatabaseConnection;

class IDatabaseTaskBatch
{
	public:

		IDatabaseTaskBatch( void ) {}
		virtual ~IDatabaseTaskBatch() {}

		virtual Loki::TypeInfo Get_Task_Type_Info( void ) const = 0;
		virtual void Add_Task( IDatabaseTask *task ) = 0;
		virtual void Add_Task( ICompoundDatabaseTask *task ) = 0;
		virtual void Execute_Tasks( IDatabaseConnection *connection, DBTaskBaseListType &successful_tasks, DBTaskBaseListType &failed_tasks ) = 0;
};

#endif // DATABASE_TASK_BATCH_H