/**********************************************************************************************************************

	DatabaseTaskBatch.h
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

#ifndef DATABASE_TASK_BATCH_H
#define DATABASE_TASK_BATCH_H

class IDatabaseTask;

template < typename T >
class TDatabaseTaskBatch
{
	public:

		TDatabaseTaskBatch( void ) :
			PendingTasks(),
			InputParameterBlock( nullptr ),
			ResultSetBlock( nullptr ),
			StatementText( L"" )
		{
			FATAL_ASSERT( T::InputParameterBatchSize > 0 && T::ResultSetBatchSize > 0 );

			InputParameterBlock = new BatchInputParametersType[ T::InputParameterBatchSize ];
			ResultSetBlock = new BatchResultSetType[ T::ResultSetBatchSize ];
		}

		~TDatabaseTaskBatch()
		{
			delete []InputParameterBlock;
			delete []ResultSetBlock;
		}

		void Add_Task( const shared_ptr< IDatabaseTask > &task ) { PendingTasks.push_back( task ); }

		void Execute_Tasks( IDatabaseConnection *connection, DBTaskListType &successful_tasks, DBTaskListType &failed_tasks )
		{
			if ( PendingTasks.size() == 0 )
			{
				return;
			}

			if ( StatementText.size() == 0 )
			{
				FATAL_ASSERT( PendingTasks[ 0 ]->Validate_Input_Signature( InputParameterBlock ) );
				StatementText = PendingTasks[ 0 ]->Get_Statement_Text( InputParameterBlock );
			}

			IDatabaseStatement *statement = connection->Allocate_Statement( StatementText );
			FATAL_ASSERT( statement != nullptr );

			statement->Bind_Input( InputParameterBlock, sizeof( BatchInputParametersType ) );
			statement->Bind_Output( ResultSetBlock, sizeof( BatchResultSetType ), T::ResultSetBatchSize );
			FATAL_ASSERT( statement->Get_Error_State() == DBEST_SUCCESS );

			while ( PendingTasks.size() > 0 )
			{
				DBTaskListType sub_list;
				uint32 range_count = 0;

				while ( PendingTasks.size() > 0 && sub_list.size() < T::InputParameterBatchSize )
				{
					sub_list.push_back( PendingTasks.front() );
					PendingTasks.pop_front();
				}

				Process_Task_List( statement, sub_list, successful_tasks, failed_tasks );
			}

			PendingTasks.clear();

			connection->Release_Statement( statement );
		}

	private:

		void Process_Task_List( IDatabaseStatement *statement, DBTaskListType &sub_list, DBTaskListType &successful_tasks, DBTaskListType &failed_tasks )
		{
			while ( sub_list.size() > 0 )
			{
				bool success = true;

				for ( DBTaskListType::iterator iter = sub_list.begin(), uint32 i = 0; iter != sub_list.end(); ++iter, ++i )
				{
					iter->Initialize_Parameters( InputParameterBlock[ i ] );
				}

				statement->Execute( sub_list.size() );
				if ( !Was_Database_Operation_Successful( statement->Get_Error_State() ) )
				{
					FATAL_ASSERT( false );	// TODO: FIX
				}

				??;

				statement->End_Transaction( success );
				if ( success )
				{
					return;
				}
			}
		}
		 
		typedef std::list< shared_ptr< IDatabaseTask > > DBTaskListType;

		typedef typename T::InputParametersType BatchInputParametersType;
		typedef typename T::ResultSetType BatchResultSetType; 

		DBTaskListType PendingTasks;

		BatchInputParametersType *InputParameterBlock;
		BatchResultSetType *ResultSetBlock;

		std::wstring StatementText;
};

#endif // DATABASE_TASK_BATCH_H