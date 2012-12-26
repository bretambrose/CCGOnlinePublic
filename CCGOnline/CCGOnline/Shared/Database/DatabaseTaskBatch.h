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

#include "Interfaces/DatabaseConnectionInterface.h"
#include "Interfaces/DatabaseStatementInterface.h"
#include "Interfaces/DatabaseTaskInterface.h"
#include "DatabaseTypes.h"
#include "Logging/LogInterface.h"
#include "EnumConversion.h"

template < typename T >
class TDatabaseTaskBatch
{
	public:

		TDatabaseTaskBatch( void ) :
			PendingTasks(),
			InputParameterBlock( nullptr ),
			ResultSetBlock( nullptr ),
			StatementText( L"" ),
			TaskName( typeid( T ).name() )
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

		void Add_Task( IDatabaseTask *task ) { PendingTasks.push_back( task ); }

		void Execute_Tasks( IDatabaseConnection *connection, DBTaskListType &successful_tasks, DBTaskListType &failed_tasks )
		{
			if ( PendingTasks.size() == 0 )
			{
				return;
			}

			if ( StatementText.size() == 0 )
			{
				IDatabaseTask *first_task = *PendingTasks.begin();
				FATAL_ASSERT( connection->Validate_Input_Signature( first_task->Get_Task_Type(), InputParameterBlock ) );
				FATAL_ASSERT( connection->Validate_Output_Signature( first_task->Get_Task_Type(), ResultSetBlock ) );
				connection->Construct_Statement_Text( first_task, InputParameterBlock, StatementText );
			}

			IDatabaseStatement *statement = connection->Allocate_Statement( StatementText );
			FATAL_ASSERT( statement != nullptr );

			if ( statement->Needs_Binding() )
			{
				statement->Bind_Input( InputParameterBlock, sizeof( BatchInputParametersType ) );
				statement->Bind_Output( ResultSetBlock, sizeof( BatchResultSetType ), T::ResultSetBatchSize );
				FATAL_ASSERT( statement->Get_Error_State() == DBEST_SUCCESS );
			}

			FATAL_ASSERT( statement->Is_Ready_For_Use() );

			LOG( LL_LOW, "DatabaseTaskBatch " << TaskName.c_str() << " - TaskCount: " << PendingTasks.size() );

			while ( PendingTasks.size() > 0 )
			{
				DBTaskListType sub_list;

				while ( PendingTasks.size() > 0 && sub_list.size() < T::InputParameterBatchSize )
				{
					sub_list.push_back( PendingTasks.front() );
					PendingTasks.pop_front();
				}

				Process_Task_List( statement, sub_list, successful_tasks, failed_tasks );
			}

			LOG( LL_LOW, "DatabaseTaskBatch " << TaskName.c_str() << " - Successes: " << successful_tasks.size() << " Failures: " << failed_tasks.size() );

			connection->Release_Statement( statement );
		}

	private:

		void Process_Task_List( IDatabaseStatement *statement, DBTaskListType &sub_list, DBTaskListType &successful_tasks, DBTaskListType &failed_tasks )
		{
			uint32 i = 0;
			for ( DBTaskListType::iterator iter = sub_list.begin(); iter != sub_list.end(); ++iter, ++i )
			{
				( *iter )->Initialize_Parameters( &InputParameterBlock[ i ] );
			}

			while ( sub_list.size() > 0 )
			{
				bool success = true;

				if ( !Was_Database_Operation_Successful( statement->Get_Error_State() ) )
				{
					FATAL_ASSERT( false );
				}

				statement->Execute( static_cast< uint32 >( sub_list.size() ) );
				if ( !Was_Database_Operation_Successful( statement->Get_Error_State() ) )
				{
					success = false;
					if ( Handle_Batch_Error( statement, sub_list, successful_tasks, failed_tasks ) )
					{
						return;
					}
				}
				else
				{
					DBTaskListType::iterator iter = sub_list.begin();

					int64 rows_fetched = 0;
					uint32 input_row = 0;
					EFetchResultsStatusType fetch_status = FRST_ONGOING;
					while ( fetch_status != FRST_ERROR && fetch_status != FRST_FINISHED_ALL )
					{
						fetch_status = FRST_ONGOING;
						while ( fetch_status == FRST_ONGOING )
						{
							fetch_status = statement->Fetch_Results( rows_fetched );
							if ( fetch_status != FRST_ERROR && rows_fetched > 0 )
							{
								( *iter )->On_Fetch_Results( ResultSetBlock, rows_fetched );
							}
						}

						if ( fetch_status == FRST_FINISHED_SET )
						{
							( *iter )->On_Fetch_Results_Finished( &InputParameterBlock[ input_row ] );
							++iter;
							++input_row;
						}
						else if ( fetch_status == FRST_FINISHED_ALL )
						{
							for ( ; iter != sub_list.end(); ++iter, ++input_row )
							{
								( *iter )->On_Fetch_Results_Finished( &InputParameterBlock[ input_row ] );
							}
						}
					}

					if ( fetch_status == FRST_FINISHED_ALL )
					{
						for ( DBTaskListType::iterator iter = sub_list.begin(); iter != sub_list.end(); ++iter )
						{
							successful_tasks.push_back( *iter );
						}

						sub_list.clear();
					}
					else
					{
						success = false;
						if ( Handle_Batch_Error( statement, sub_list, successful_tasks, failed_tasks ) )
						{
							return;
						}
					}
				}

				statement->End_Transaction( success );
				if ( success )
				{
					return;
				}
			}
		}

		bool Handle_Batch_Error( IDatabaseStatement *statement, DBTaskListType &sub_list, DBTaskListType &successful_tasks, DBTaskListType &failed_tasks )
		{
			if ( sub_list.size() == 1 )
			{
				IDatabaseTask *task = *( sub_list.begin() );
				Log_Error( statement, &InputParameterBlock[ 0 ] );

				statement->End_Transaction( false );
				failed_tasks.push_back( task );
				sub_list.clear();
				return true;
			}

			int32 bad_row_number = statement->Get_Bad_Row_Number();
			if ( bad_row_number < 0 )
			{
				Log_Error( statement, nullptr );

				statement->End_Transaction( false );
				Process_Task_List_One_By_One( statement, sub_list, successful_tasks, failed_tasks );
				return true;
			}
			else
			{						
				int32 row = 0;
				for( DBTaskListType::iterator iter = sub_list.begin(); iter != sub_list.end(); ++row, ++iter )
				{
					( *iter )->On_Rollback();

					if ( row >= bad_row_number )
					{
						if ( row == bad_row_number )
						{
							Log_Error( statement, &InputParameterBlock[ row ] );

							failed_tasks.push_back( *iter );
							iter = sub_list.erase( iter );
						}

						( *iter )->Initialize_Parameters( &InputParameterBlock[ row ] );
					} 
				}

				return false;
			}
		}

		void Process_Task_List_One_By_One( IDatabaseStatement *statement, DBTaskListType &sub_list, DBTaskListType &successful_tasks, DBTaskListType &failed_tasks )
		{
			for ( DBTaskListType::iterator iter = sub_list.begin(); iter != sub_list.end(); ++iter )
			{
				( *iter )->On_Rollback();
			}

			DBTaskListType individual_list;
			for ( DBTaskListType::iterator iter = sub_list.begin(); iter != sub_list.end(); ++iter )
			{
				individual_list.clear();
				individual_list.push_back( *iter );

				Process_Task_List( statement, individual_list, successful_tasks, failed_tasks );
			}

			sub_list.clear();
		}

		void Log_Error( IDatabaseStatement *statement, IDatabaseVariableSet *input_params )
		{
			FATAL_ASSERT( statement != nullptr );

			statement->Log_Error_State();

			if ( input_params != nullptr )
			{
				std::vector< IDatabaseVariable * > params;
				input_params->Get_Variables( params );
				if ( params.size() > 0 )
				{
					LOG( LL_LOW, "\tDumping Parameters:" );
					for ( uint32 i = 0; i < params.size(); ++i )
					{
						std::string value_string;
						Convert_Database_Variable_To_String( params[ i ], value_string );

						std::string type_string;
						CEnumConverter::Convert< EDatabaseVariableValueType >( params[ i ]->Get_Value_Type(), type_string );

						std::string param_type_string;
						CEnumConverter::Convert< EDatabaseVariableType >( params[ i ]->Get_Parameter_Type(), param_type_string );

						LOG( LL_LOW, "\t\t" << i << ": " << value_string.c_str() << "(" << type_string.c_str() << ", " << param_type_string.c_str() << ")" );
					}
				}
			}
			else
			{
				LOG( LL_LOW, "\tUnable to determine which task invocation created this error." );
			}
		}
		 
		typedef typename T::InputParametersType BatchInputParametersType;
		typedef typename T::ResultSetType BatchResultSetType; 

		DBTaskListType PendingTasks;

		BatchInputParametersType *InputParameterBlock;
		BatchResultSetType *ResultSetBlock;

		std::wstring StatementText;
		std::string TaskName;
};

#endif // DATABASE_TASK_BATCH_H