/**********************************************************************************************************************

	CompoundDatabaseTaskBatch.h
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

#ifndef COMPOUND_DATABASE_TASK_BATCH_H
#define COMPOUND_DATABASE_TASK_BATCH_H

#include "Interfaces/CompoundDatabaseTaskBatchInterface.h"
#include "DatabaseCallContext.h"

class ICompoundDatabaseTask;

template< typename T >
class TCompoundDatabaseTaskBatch : public ICompoundDatabaseTaskBatch
{
	public:

		typedef IDatabaseTaskBatch BASECLASS;

		TCompoundDatabaseTaskBatch( void ) :
			BASECLASS(),
			ChildOrdering(),
			ChildTypes(),
			PendingTasks(),
			TaskName( typeid( T ).name() )
		{
			T::Register_Child_Tasks( this );
		}

		virtual ~TCompoundDatabaseTaskBatch()
		{
			FATAL_ASSERT( PendingTasks.size() == 0 );

			for ( auto iter = ChildTypes.begin(); iter != ChildTypes.end(); ++iter )
			{
				delete iter->second;
			}

			ChildTypes.clear();
			ChildOrdering.clear();
		}

		virtual Loki::TypeInfo Get_Task_Type_Info( void ) const
		{
			return Loki::TypeInfo( typeid( T ) );
		}

		virtual void Add_Task( ICompoundDatabaseTask *task ) { PendingTasks.push_back( task ); }

		virtual void Execute_Tasks( IDatabaseConnection *connection, DBTaskListType &successful_tasks, DBTaskListType &failed_tasks )
		{
			if ( PendingTasks.size() == 0 )
			{
				return;
			}

			successful_tasks.clear();
			failed_tasks.clear();

			LOG( LL_LOW, "DatabaseTaskBatch " << TaskName.c_str() << " - TaskCount: " << PendingTasks.size() );

			while ( PendingTasks.size() > 0 )
			{
				DBCompoundTaskListType sub_list;

				while ( PendingTasks.size() > 0 && sub_list.size() < T::TaskBatchSize )
				{
					sub_list.push_back( PendingTasks.front() );
					PendingTasks.pop_front();
				}

				Process_Parent_Task_List( connection, sub_list, successful_tasks, failed_tasks );
			}

			LOG( LL_LOW, "DatabaseTaskBatch " << TaskName.c_str() << " - Successes: " << successful_tasks.size() << " Failures: " << failed_tasks.size() );
		}

		virtual void Register_Child_Variable_Sets( const Loki::TypeInfo &type_info, IDatabaseCallContext *child_call_context )
		{
			FATAL_ASSERT( ChildTypes.find( type_info ) == ChildTypes.end() );
			ChildOrdering.push_back( type_info );

			ChildTypes.insert( ChildTypeTable::value_type( type_info, child_call_context ) );
		}

	private:

		void Process_Parent_Task_List( IDatabaseConnection *connection, DBCompoundTaskListType &sub_list, DBTaskListType &successful_tasks, DBTaskListType &failed_tasks )
		{
			while ( sub_list.size() > 0 )
			{
				for ( auto iter = sub_list.cbegin(); iter != sub_list.cend(); ++iter )
				{
					( *iter )->Clear_Child_Tasks();
					( *iter )->Seed_Child_Tasks();
				}

				DatabaseTaskIDType::Enum bad_task = DatabaseTaskIDType::INVALID;
				ExecuteDBTaskListResult::Enum process_child_result = ExecuteDBTaskListResult::SUCCESS;

				for ( auto tt_iter = ChildOrdering.cbegin(); tt_iter != ChildOrdering.cend(); ++tt_iter )
				{
					DBTaskListType child_list;
					for ( auto iter = sub_list.cbegin(); iter != sub_list.cend(); ++iter )
					{
						( *iter )->Get_Child_Tasks_Of_Type( *tt_iter, child_list );
					}

					if( child_list.size() == 0 )
					{
						continue;
					}

					auto context_iter = ChildCallContexts.find( *tt_iter );
					FATAL_ASSERT( context_iter != ChildCallContexts.end() );

					IDatabaseCallContext *call_context = context_iter->second;

					ExecuteDBTaskListResult::Enum process_child_result = Process_Child_Task_List( call_context, connection, *tt_iter, child_list, bad_task );
					if ( process_child_result == ExecuteDBTaskListResult::SUCCESS )
					{
						std::for_each( child_list.begin(), child_list.end(), [&tt_iter] ( DBTaskListType::iterator iter ) { ( *iter )->On_Child_Task_Type_Success( tt_iter );});
					}
					else
					{
						break;
					}
				}

				// commit/rollback etc...
				if(process_child_result == ExecuteDBTaskListResult::SUCCESS)
				{
					connection->End_Transaction( true );
					successful_tasks.splice( successful_tasks.end(), sub_list );
				}
				else
				{
					connection->End_Transaction( false );

					if ( sub_list.size() == 1 )
					{
						failed_tasks.splice( failed_tasks.end(), sub_list );
						return;
					}

					if ( bad_task != DatabaseTaskIDType::INVALID )
					{
						auto find_iter = std::find( sub_list.begin(), sub_list.end(), [bad_task]( DBTaskListType::iterator s_iter ){ return ( *s_iter )->Get_ID() == bad_task; } );
						if ( find_iter != sub_list.end() )
						{
							failed_tasks.push_back( *find_iter );
							sub_list.erase( find_iter );
						}
						else
						{
							bad_task = DatabaseTaskIDType::INVALID;
						}
					}

					if ( bad_task == DatabaseTaskIDType::INVALID )
					{
						for ( auto iter = sub_list.begin(); iter != sub_list.end(); ++iter )
						{
							DBCompoundTaskListType new_sub_list;
							new_sub_list.push_back( *iter );
							Process_Parent_Task_List( connection, new_sub_list, successful_tasks, failed_tasks );
						}
						
						return;						
					}
				}
			}
		}

		ExecuteDBTaskListResult::Enum Process_Child_Task_List( IDatabaseCallContext *call_context, IDatabaseConnection *connection, const DBTaskListType &child_list, DatabaseTaskIDType::Enum &bad_task_id )
		{
			if ( call_context->Get_Statement_Text().size() == 0 )
			{
				IDatabaseTask *first_task = *child_list.begin();
				FATAL_ASSERT( connection->Validate_Input_Output_Signatures( first_task, call_context->Get_Param_Rows(), call_context->Get_Result_Rows() ) );

				std::wstring statement_text;
				connection->Construct_Statement_Text( first_task, call_context->Get_Param_Rows(), statement_text );

				call_context->Set_Statement_Text( statement_text );
			}

			IDatabaseStatement *statement = connection->Allocate_Statement( call_context->Get_Statement_Text() );
			FATAL_ASSERT( statement != nullptr );

			if ( statement->Needs_Binding() )
			{
				statement->Bind_Input( call_context->Get_Param_Rows(), call_context->Get_Sizeof_Param_Type() );
				statement->Bind_Output( call_context->Get_Result_Rows(), call_context->Get_Sizeof_Result_Type(), call_context->Get_Result_Row_Count() );
			}

			FATAL_ASSERT( statement->Is_Ready_For_Use() );

			// LOG( LL_LOW, "DatabaseTaskBatch " << TaskName.c_str() << " - TaskCount: " << PendingTasks.size() );

			DBTaskListType child_list_copy;
			std::copy( child_list.begin(), child_list.end(), back_inserter(child_list_copy) );

			while ( child_list_copy.size() > 0 )
			{
				DBTaskListType sub_list;

				while ( child_list_copy.size() > 0 && sub_list.size() < call_context->Get_Param_Row_Count() )
				{
					sub_list.push_back( child_list_copy.front() );
					child_list_copy.pop_front();
				}

				ExecuteDBTaskListResult::Enum result = ExecuteDBTaskListResult::SUCCESS;
				DBTaskListType::const_iterator failure_iter = sub_list.end();
				
				DBUtils::Execute_Task_List( call_context, statement, sub_list, result, failure_iter );
				if ( result != ExecuteDBTaskListResult::SUCCESS )
				{
					if ( result == ExecuteDBTaskListResult::FAILED_SPECIFIC_TASK )
					{
						bad_task_id = ( *failure_iter )->Get_ID();
					}

					return result;
				}
			}

			// LOG( LL_LOW, "DatabaseTaskBatch " << TaskName.c_str() << " - Successes: " << successful_tasks.size() << " Failures: " << failed_tasks.size() );

			FATAL_ASSERT( statement->Is_Ready_For_Use() );

			connection->Release_Statement( statement );
		}

		typedef std::vector< Loki::TypeInfo > ChildTypeVector;
		typedef stdext::hash_map< Loki::TypeInfo, IDatabaseCallContext *, STypeInfoHelper > ChildCallContextTable;

		ChildTypeVector ChildOrdering;
		ChildCallContextTable ChildCallContexts;

		DBCompoundTaskListType PendingTasks;

		std::string TaskName;
};


template< typename T >
void Register_Database_Child_Task_Type( ICompoundDatabaseTaskBatch *compound_batch )
{
	uint32 params_batch_size = T::InputParameterBatchSize;
	FATAL_ASSERT( params_batch_size > 0 );

	uint32 result_set_batch_size = T::ResultSetBatchSize;
	FATAL_ASSERT( result_set_batch_size > 0 );

   IDatabaseCallContext *child_call_context = new CDatabaseCallContext< T::InputParametersType, params_batch_size, T::ResultSetType, result_set_batch_size >();

	compound_batch->Register_Child_Variable_Sets( Loki::TypeInfo( typeid( T ) ), child_call_context );
}

#endif // COMPOUND_DATABASE_TASK_BATCH_H