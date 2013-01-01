/**********************************************************************************************************************

	DatabaseCalls.h
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

#ifndef DATABASE_CALLS_H
#define DATABASE_CALLS_H

#include "Interfaces/DatabaseTaskInterface.h"
#include "DatabaseTypes.h"

class CEmptyVariableSet;

template < typename I, uint32 ISIZE >
class TDatabaseFunctionCall : public IDatabaseTask
{
	public:

		typedef IDatabaseTask BASECLASS;

		TDatabaseFunctionCall( void ) :
			BASECLASS()
		{}

		virtual ~TDatabaseFunctionCall() {}

		virtual EDatabaseTaskType Get_Task_Type( void ) const { return DTT_FUNCTION_CALL; }

		typedef I InputParametersType;
		typedef CEmptyVariableSet ResultSetType;

		static const uint32 InputParameterBatchSize = ISIZE;
		static const uint32 ResultSetBatchSize = 1;

	protected:

		virtual void On_Fetch_Results( IDatabaseVariableSet * /*result_set*/, int64 rows_fetched ) {
			ASSERT_TRUE( rows_fetched == 0 );
		}
};

template < typename I, uint32 ISIZE, typename O, uint32 OSIZE >
class TDatabaseProcedureCall : public IDatabaseTask
{
	public:

		typedef IDatabaseTask BASECLASS;

		TDatabaseProcedureCall( void ) :
			BASECLASS()
		{}

		virtual ~TDatabaseProcedureCall() {}

		virtual EDatabaseTaskType Get_Task_Type( void ) const { return DTT_PROCEDURE_CALL; }

		typedef I InputParametersType;
		typedef O ResultSetType;

		static const uint32 InputParameterBatchSize = ISIZE;
		static const uint32 ResultSetBatchSize = OSIZE;
};

#endif // DATABASE_CALLS_H