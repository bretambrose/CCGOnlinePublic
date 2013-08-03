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

#include "Interfaces/DatabaseTaskBatchInterface.h"
#include "Interfaces/DatabaseConnectionInterface.h"
#include "Interfaces/DatabaseStatementInterface.h"
#include "Interfaces/DatabaseTaskInterface.h"
#include "DatabaseTaskBatchUtilities.h"
#include "DatabaseTypes.h"
#include "DatabaseCallContext.h"
#include "Logging/LogInterface.h"

template < typename T >
class TDatabaseTaskBatch : public IDatabaseTaskBatch
{
	public:

		typedef IDatabaseTaskBatch BASECLASS;

		TDatabaseTaskBatch( void ) :
			BASECLASS(),
			PendingTasks(),
			CallContext( nullptr ),
			TaskName( typeid( T ).name() )
		{
			FATAL_ASSERT( T::InputParameterBatchSize > 0 && T::ResultSetBatchSize > 0 );

			CallContext = new CDatabaseCallContext< T::InputParametersType, T::InputParameterBatchSize, T::ResultSetType, T::ResultSetBatchSize >();
		}

		virtual ~TDatabaseTaskBatch()
		{
			FATAL_ASSERT( PendingTasks.size() == 0 );

			delete CallContext;
			CallContext = nullptr;
		}

		virtual Loki::TypeInfo Get_Task_Type_Info( void ) const
		{
			return Loki::TypeInfo( typeid( T ) );
		}

		virtual void Add_Task( IDatabaseTask *task ) { PendingTasks.push_back( task ); }
		virtual void Add_Task( ICompoundDatabaseTask * /*task*/ ) { FATAL_ASSERT( false ); }

		virtual void Execute_Tasks( IDatabaseConnection *connection, DBTaskBaseListType &successful_tasks, DBTaskBaseListType &failed_tasks )
		{
			if ( PendingTasks.size() == 0 )
			{
				return;
			}

			if ( CallContext->Get_Statement_Text().size() == 0 )
			{
				IDatabaseTask *first_task = *PendingTasks.begin();
				FATAL_ASSERT( connection->Validate_Input_Output_Signatures( first_task, CallContext->Get_Param_Rows(), CallContext->Get_Result_Rows() ) );

				std::wstring statement_text;
				connection->Construct_Statement_Text( first_task, CallContext->Get_Param_Rows(), statement_text );

				CallContext->Set_Statement_Text( statement_text );
			}

			IDatabaseStatement *statement = connection->Allocate_Statement( CallContext->Get_Statement_Text() );
			FATAL_ASSERT( statement != nullptr );

			if ( statement->Needs_Binding() )
			{
				statement->Bind_Input( CallContext->Get_Param_Rows(), sizeof(T::InputParametersType) );
				statement->Bind_Output( CallContext->Get_Result_Rows(), sizeof(T::ResultSetType), T::ResultSetBatchSize );
			}

			FATAL_ASSERT( statement->Is_Ready_For_Use() );

			LOG( LL_LOW, "DatabaseTaskBatch " << TaskName.c_str() << " - TaskCount: " << PendingTasks.size() );

			while ( PendingTasks.size() > 0 )
			{
				DBTaskListType sub_list;

				auto splice_iter = PendingTasks.begin();
				uint32 advance_amount = std::min<uint32>( static_cast< uint32 >( PendingTasks.size() ), T::InputParameterBatchSize );
				std::advance( splice_iter, advance_amount );
				sub_list.splice( sub_list.end(), PendingTasks, PendingTasks.begin(), splice_iter );

				Process_Task_List( statement, sub_list, successful_tasks, failed_tasks );
			}

			LOG( LL_LOW, "DatabaseTaskBatch " << TaskName.c_str() << " - Successes: " << successful_tasks.size() << " Failures: " << failed_tasks.size() );

			FATAL_ASSERT( statement->Is_Ready_For_Use() );

			connection->Release_Statement( statement );
		}

	private:

		void Process_Task_List( IDatabaseStatement *statement, DBTaskListType &sub_list, DBTaskBaseListType &successful_tasks, DBTaskBaseListType &failed_tasks )
		{
			while( sub_list.size() > 0 )
			{
				ExecuteDBTaskListResult::Enum execute_result;
				DBTaskListType::const_iterator bad_task;
				DBUtils::Execute_Task_List( CallContext, statement, sub_list, execute_result, bad_task );
				if ( execute_result == ExecuteDBTaskListResult::SUCCESS )
				{
					statement->Get_Connection()->End_Transaction( true );
					statement->Return_To_Ready();

					std::for_each( sub_list.begin(), sub_list.end(), [ &successful_tasks ]( IDatabaseTask *task ){ successful_tasks.push_back( task ); } );
					sub_list.clear();
				}
				else
				{
					statement->Get_Connection()->End_Transaction( false );
					statement->Return_To_Ready();

					for ( DBTaskListType::iterator iter = sub_list.begin(); iter != sub_list.end(); ++iter )
					{
						if ( iter != bad_task )
						{
							( *iter )->On_Rollback();
						}
					}

					if ( execute_result == ExecuteDBTaskListResult::FAILED_SPECIFIC_TASK )
					{
						failed_tasks.push_back( *bad_task );
						sub_list.erase( bad_task );
					}
					else
					{
						DBTaskListType individual_list;
						for ( DBTaskListType::iterator iter = sub_list.begin(); iter != sub_list.end(); ++iter )
						{
							individual_list.clear();
							individual_list.push_back( *iter );

							Process_Task_List( statement, individual_list, successful_tasks, failed_tasks );
						}

						sub_list.clear();
					}
				}
			}
		}

		DBTaskListType PendingTasks;

		IDatabaseCallContext *CallContext;

		std::string TaskName;
};

#endif // DATABASE_TASK_BATCH_H