/**********************************************************************************************************************

	DatabaseTask.h
		A component defining 

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

#ifndef DATABASE_TASK_H
#define DATABASE_TASK_H

#include "Interfaces/DatabaseTaskInterface.h"

template < typename I, uint32 ISIZE, typename O, uint32 OSIZE >
class TDatabaseTask : public IDatabaseTask
{
	public:

		typedef IDatabaseTask BASECLASS;

		TDatabaseTask( void ) {}
		virtual ~TDatabaseTask() {}

		typedef I InputParametersType;
		typedef O ResultSetType;

		static const uint32 InputParameterBatchSize = ISIZE;
		static const uint32 ResultSetBatchSize = OSIZE;
};

#endif // DATABASE_TASK_H