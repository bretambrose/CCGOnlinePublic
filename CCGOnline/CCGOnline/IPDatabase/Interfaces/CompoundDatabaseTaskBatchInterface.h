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

#ifndef COMPOUND_DATABASE_TASK_BATCH_INTERFACE_H
#define COMPOUND_DATABASE_TASK_BATCH_INTERFACE_H

#include "DatabaseTaskBatchInterface.h"

class IDatabaseCallContext;

class ICompoundDatabaseTaskBatch : public IDatabaseTaskBatch
{
	public:

		typedef IDatabaseTaskBatch BASECLASS;

		ICompoundDatabaseTaskBatch( void ) :
			BASECLASS()
		{}

		virtual ~ICompoundDatabaseTaskBatch() {}

		virtual void Register_Child_Variable_Sets( const Loki::TypeInfo &type_info, IDatabaseCallContext *child_call_context ) = 0;
};

#endif // COMPOUND_DATABASE_TASK_BATCH_INTERFACE_H