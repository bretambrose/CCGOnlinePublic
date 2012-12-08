/**********************************************************************************************************************

	DatabaseTaskInterface.h
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

#ifndef DATABASE_TASK_INTERFACE_H
#define DATABASE_TASK_INTERFACE_H

class IDatabaseVariableSet;

class IDatabaseTask
{
	public:
		
		IDatabaseTask( void );
		virtual ~IDatabaseTask() {}

	protected:

		friend template < typename T > class TDatabaseTaskBatch;

		// WIP: signatures in flux
		virtual void Initialize_Parameters( IDatabaseVariableSet *input_parameters ) = 0;		
		virtual void On_Fetch_Results( IDatabaseVariableSet *result_set, int64 rows_fetched ) = 0;			
		virtual void On_Fetch_Results_Finished( IDatabaseVariableSet *input_parameters ) = 0;	

		virtual void On_Task_Success( void ) = 0;					
		virtual void On_Task_Failure( void ) = 0;					// error information needs to be copied into a base class 

		virtual const wchar_t *Get_Statement_Text( IDatabaseVariableSet *input_parameters ) = 0;	// implemented in DBProcedureCall, FunctionCall

		virtual bool Validate_Input_Signature( IDatabaseVariableSet *input_parameters ) = 0;		// called only once ever per TaskBatch, implemented in DBProcedureCall, FunctionCall

};

#endif // DATABASE_TASK_INTERFACE_H