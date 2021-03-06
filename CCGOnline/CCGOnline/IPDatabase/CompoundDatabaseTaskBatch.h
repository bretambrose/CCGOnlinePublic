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

#include "Interfaces/CompoundDatabaseTaskBatchInterface.h"
#include "Interfaces/DatabaseTaskBaseInterface.h"
#include "DatabaseCallContext.h"
#include "DatabaseTaskBatchUtilities.h"
#include "IPShared/Logging/LogInterface.h"

namespace IP
{
namespace Db
{

class ICompoundDatabaseTask;

template< typename T >
class TCompoundDatabaseTaskBatch : public ICompoundDatabaseTaskBatch
{
	public:

		using BASECLASS = ICompoundDatabaseTaskBatch;

		TCompoundDatabaseTaskBatch( void ) :
			BASECLASS(),
			ChildOrdering(),
			ChildCallContexts(),
			PendingTasks(),
			TaskName( typeid( T ).name() )
		{
			T::Register_Child_Tasks( this );
		}

		virtual ~TCompoundDatabaseTaskBatch()
		{
			FATAL_ASSERT( PendingTasks.empty() );

			std::for_each( ChildCallContexts.begin(), ChildCallContexts.end(), []( ChildCallContextPair &pair ){ delete pair.second; } );

			ChildCallContexts.clear();
			ChildOrdering.clear();
		}

		virtual Loki::TypeInfo Get_Task_Type_Info( void ) const
		{
			return Loki::TypeInfo( typeid( T ) );
		}

		virtual void Add_Task( IDatabaseTask * /*task*/ ) { FATAL_ASSERT( false ); }
		virtual void Add_Task( ICompoundDatabaseTask *task ) { PendingTasks.push_back( static_cast< ICompoundDatabaseTask* >( task ) ); }

		virtual void Execute_Tasks( IDatabaseConnection *connection, DBTaskBaseListType &successful_tasks, DBTaskBaseListType &failed_tasks )
		{
			successful_tasks.clear();
			failed_tasks.clear();

			if ( PendingTasks.empty() )
			{
				return;
			}

			LOG( IP::Logging::ELogLevel::LL_LOW, "CompoundDatabaseTaskBatch " << TaskName.c_str() << " - TaskCount: " << PendingTasks.size() );

			while ( !PendingTasks.empty() )
			{
				DBCompoundTaskListType sub_list;

				auto splice_iter = PendingTasks.begin();
				std::advance( splice_iter, std::min( static_cast< uint32_t >( PendingTasks.size() ), T::CompoundTaskBatchSize ) );
				sub_list.splice( sub_list.end(), PendingTasks, PendingTasks.begin(), splice_iter );

				Process_Parent_Task_List( connection, sub_list, successful_tasks, failed_tasks );
			}

			LOG( IP::Logging::ELogLevel::LL_LOW, "CompoundDatabaseTaskBatch " << TaskName.c_str() << " - Successes: " << successful_tasks.size() << ", Failures: " << failed_tasks.size() );
		}

		virtual void Register_Child_Variable_Sets( const Loki::TypeInfo &type_info, IDatabaseCallContext *child_call_context )
		{
			FATAL_ASSERT( ChildCallContexts.find( type_info ) == ChildCallContexts.end() );
			ChildOrdering.push_back( type_info );

			ChildCallContexts[ type_info ] = child_call_context;
		}

	private:

		void Process_Parent_Task_List( IDatabaseConnection *connection, DBCompoundTaskListType &sub_list, DBTaskBaseListType &successful_tasks, DBTaskBaseListType &failed_tasks )
		{
			while ( !sub_list.empty() )
			{
				std::for_each( sub_list.cbegin(), sub_list.cend(), []( ICompoundDatabaseTask *task ) { task->Clear_Child_Tasks(); task->Seed_Child_Tasks(); } );

				EDatabaseTaskIDType bad_task = EDatabaseTaskIDType::INVALID;
				EExecuteDBTaskListResult process_child_result = EExecuteDBTaskListResult::SUCCESS;

				for ( auto tt_iter = ChildOrdering.cbegin(), end = ChildOrdering.cend(); tt_iter != end; ++tt_iter )
				{
					DBTaskListType child_list;
					std::for_each( sub_list.cbegin(), sub_list.cend(), [ &tt_iter, &child_list ]( ICompoundDatabaseTask *task ) { task->Get_Child_Tasks_Of_Type( *tt_iter, child_list ); } );

					if( child_list.empty() )
					{
						continue;
					}

					LOG( IP::Logging::ELogLevel::LL_LOW, "CompoundDatabaseTaskBatch " << TaskName.c_str() << ", ChildTask " << tt_iter->name() << " - TaskCount: " << child_list.size() );

					auto context_iter = ChildCallContexts.find( *tt_iter );
					FATAL_ASSERT( context_iter != ChildCallContexts.end() );

					IDatabaseCallContext *call_context = context_iter->second;

					process_child_result = Process_Child_Task_List( call_context, connection, child_list, bad_task );
					if ( process_child_result != EExecuteDBTaskListResult::SUCCESS )
					{
						break;
					}
					
					std::for_each( sub_list.begin(), sub_list.end(), [&tt_iter] ( ICompoundDatabaseTask *task ) { task->On_Child_Task_Success( *tt_iter );});
				}

				// commit/rollback etc...
				if(process_child_result == EExecuteDBTaskListResult::SUCCESS)
				{
					connection->End_Transaction( true );

					// can't use splice due to type difference
					std::for_each( sub_list.begin(), sub_list.end(), [ &successful_tasks ]( ICompoundDatabaseTask *task ){ successful_tasks.push_back( task ); } );
					sub_list.clear();
				}
				else
				{
					connection->End_Transaction( false );

					if ( sub_list.size() == 1 )
					{
						failed_tasks.push_back( *( sub_list.begin() ) );
						sub_list.clear();
						return;
					}

					if ( bad_task != EDatabaseTaskIDType::INVALID )
					{
						auto find_iter = std::find_if( sub_list.begin(), sub_list.end(), [bad_task]( ICompoundDatabaseTask *task ){ return task->Get_ID() == bad_task; } );
						if ( find_iter != sub_list.end() )
						{
							failed_tasks.push_back( *find_iter );
							sub_list.erase( find_iter );
						}
						else
						{
							bad_task = EDatabaseTaskIDType::INVALID;
						}
					}

					if ( bad_task == EDatabaseTaskIDType::INVALID )
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

		EExecuteDBTaskListResult Process_Child_Task_List( IDatabaseCallContext *call_context, IDatabaseConnection *connection, const DBTaskListType &child_list, EDatabaseTaskIDType &bad_task_id )
		{
			if ( child_list.empty() )
			{
				return EExecuteDBTaskListResult::SUCCESS;
			}

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

			DBTaskListType child_list_copy;
			std::copy( child_list.begin(), child_list.end(), back_inserter(child_list_copy) );

			while ( !child_list_copy.empty() )
			{
				DBTaskListType sub_list;

				auto end_of_splice_iter = child_list_copy.begin();
				std::advance( end_of_splice_iter, std::min( static_cast<uint32_t>(child_list_copy.size()), call_context->Get_Param_Row_Count() ) );
				sub_list.splice( sub_list.end(), child_list_copy, child_list_copy.begin(), end_of_splice_iter );

				EExecuteDBTaskListResult result = EExecuteDBTaskListResult::SUCCESS;
				DBTaskListType::const_iterator failure_iter = sub_list.end();
				
				Execute_Task_List( call_context, statement, sub_list, result, failure_iter );
				if ( result != EExecuteDBTaskListResult::SUCCESS )
				{
					if ( result == EExecuteDBTaskListResult::FAILED_SPECIFIC_TASK )
					{
						bad_task_id = ( *failure_iter )->Get_ID();
					}

					return result;
				}
			}

			FATAL_ASSERT( statement->Is_Ready_For_Use() );

			connection->Release_Statement( statement );

			return EExecuteDBTaskListResult::SUCCESS;
		}

		using ChildTypeVector = std::vector< Loki::TypeInfo >;
		using ChildCallContextTable = std::unordered_map< Loki::TypeInfo, IDatabaseCallContext *, STypeInfoContainerHelper >;
		using ChildCallContextPair = std::pair< const Loki::TypeInfo, IDatabaseCallContext * >;

		ChildTypeVector ChildOrdering;
		ChildCallContextTable ChildCallContexts;

		DBCompoundTaskListType PendingTasks;

		std::string TaskName;
};


template< typename T >
void Register_Database_Child_Task_Type( ICompoundDatabaseTaskBatch *compound_batch )
{
   IDatabaseCallContext *child_call_context = new CDatabaseCallContext< T::InputParametersType, T::InputParameterBatchSize, T::ResultSetType, T::ResultSetBatchSize >();

	compound_batch->Register_Child_Variable_Sets( Loki::TypeInfo( typeid( T ) ), child_call_context );
}

} // namespace Db
} // namespace IP