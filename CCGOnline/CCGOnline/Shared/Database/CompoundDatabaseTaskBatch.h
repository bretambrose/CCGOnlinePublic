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

#ifdef NEVER

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

		virtual void Add_Task( IDatabaseTask *task ) { PendingTasks.push_back( task ); }

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
				DBTaskListType sub_list;

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

		void Process_Parent_Task_List( IDatabaseConnection *connection, DBTaskListType &sub_list, DBTaskListType &successful_tasks, DBTaskListType &failed_tasks )
		{
			for ( auto iter = sub_list.cbegin(); iter != sub_list.cend(); ++iter )
			{
				( *iter )->Clear_Child_Tasks();
				( *iter )->Seed_Child_Tasks();
			}

			DatabaseTaskIDType::Enum bad_task = DatabaseTaskIDType::INVALID;
			connection->
			??;

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

				bool success = Process_Child_Task_List( connection, *tt_iter, child_list, bad_task );
				if ( !success )
				{
					??;
				}
			}

			// commit/rollback etc...
			??;
		}

		bool Process_Child_Task_List( IDatabaseConnection *connection, const Loki::TypeInfo &task_type, const DBTaskListType &child_list, DatabaseTaskIDType::Enum &bad_task_id )
		{
			??;
		}

		typedef std::vector< Loki::TypeInfo > ChildTypeVector;
		typedef stdext::hash_map< Loki::TypeInfo, IDatabaseCallContext *, STypeInfoHelper > ChildCallContextTable;

		ChildTypeVector ChildOrdering;
		ChildCallContextTable ChildCallContexts;

		DBTaskListType PendingTasks;

		std::string TaskName;
};

#endif // NEVER

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