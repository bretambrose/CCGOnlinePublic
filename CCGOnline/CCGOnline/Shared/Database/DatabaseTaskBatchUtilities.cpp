/**********************************************************************************************************************

	DatabaseTaskBatchUtilities.cpp
		A component containing 

	(c) Copyright 2011, Bret Ambrose (mailto:bretambrose@gmail.com).

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

#include "stdafx.h"

#include "DatabaseTaskBatchUtilities.h"

#include "Interfaces/DatabaseCallContextInterface.h"
#include "Interfaces/DatabaseStatementInterface.h"
#include "Interfaces/DatabaseTaskInterface.h"
#include "Interfaces/DatabaseVariableInterface.h"
#include "Interfaces/DatabaseVariableSetInterface.h"
#include "Logging/LogInterface.h"
#include "EnumConversion.h"

static void Log_DB_Error( IDatabaseStatement *statement, IDatabaseVariableSet *input_params, const wchar_t *additional_error_context )
{
	FATAL_ASSERT( statement != nullptr );

	LOG( LL_LOW, "\tBATCH PROCESSING ERROR" );
	if ( additional_error_context != nullptr )
	{
		WLOG( LL_LOW, additional_error_context );
	}
			 
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
				input_params->Convert_Variable_To_String( params[ i ], value_string );

				std::string type_string;
				CEnumConverter::Convert< EDatabaseVariableValueType >( params[ i ]->Get_Value_Type(), type_string );

				std::string param_type_string;
				CEnumConverter::Convert< EDatabaseVariableType >( params[ i ]->Get_Parameter_Type(), param_type_string );

				LOG( LL_LOW, "\t\t" << i << ": " << value_string.c_str() << "(" << type_string.c_str() << ", " << param_type_string.c_str() << ")" );
			}
		}
		else
		{
			LOG( LL_LOW, "\tNo Input Parameters." );
		}
	}
	else
	{
		LOG( LL_LOW, "\tUnable to determine which task invocation created this error." );
	}
}

static void Extract_Batch_Error( IDatabaseCallContext *call_context, 
											IDatabaseStatement *statement, 
											const wchar_t *additional_error_context, 
											const DBTaskListType &sub_list, 
											ExecuteDBTaskListResult::Enum &result, 
											DBTaskListType::const_iterator &first_failed_task )
{
	FATAL_ASSERT( sub_list.size() > 0 );

	if ( sub_list.size() == 1 )
	{
		Log_DB_Error( statement, call_context->Get_Param_Row( 0 ), additional_error_context );
		result = ExecuteDBTaskListResult::FAILED_SPECIFIC_TASK;
		first_failed_task = sub_list.cbegin();
		return;
	}

	int32 bad_row_number = statement->Get_Bad_Row_Number();
	if ( bad_row_number >= 0 )
	{
		int32 row = 0;
		for( first_failed_task = sub_list.cbegin(); first_failed_task != sub_list.cend(); ++row, ++first_failed_task )
		{
			if ( row == bad_row_number )
			{
				Log_DB_Error( statement, call_context->Get_Param_Row( row ), additional_error_context );
				result = ExecuteDBTaskListResult::FAILED_SPECIFIC_TASK;
				return;
			}
		}
	}

	Log_DB_Error( statement, nullptr, additional_error_context );
	result = ExecuteDBTaskListResult::FAILED_UNKNOWN_TASK;
}

void DBUtils::Execute_Task_List( IDatabaseCallContext *call_context, IDatabaseStatement *statement, const DBTaskListType &sub_list, ExecuteDBTaskListResult::Enum &result, DBTaskListType::const_iterator &first_failed_task )
{
	FATAL_ASSERT( call_context != nullptr );
	FATAL_ASSERT( statement != nullptr );

	result = ExecuteDBTaskListResult::SUCCESS;
	first_failed_task = sub_list.cend();

	if ( sub_list.size() == 0 )
	{
		return;
	}

	uint32 i = 0;
	for ( DBTaskListType::const_iterator iter = sub_list.cbegin(); iter != sub_list.cend(); ++iter, ++i )
	{
		( *iter )->Initialize_Parameters( call_context->Get_Param_Row( i ) );
	}

	FATAL_ASSERT( !statement->Is_In_Error_State() );
	size_t list_size = sub_list.size();

	statement->Execute( static_cast< uint32 >( sub_list.size() ) );
	if ( statement->Is_In_Error_State() )
	{
		Extract_Batch_Error( call_context, statement, NULL, sub_list, result, first_failed_task );
		return;
	}

	DBTaskListType::const_iterator iter = sub_list.cbegin();

	int64 rows_fetched = 0;
	uint32 input_row = 0;
	EFetchResultsStatusType fetch_status = FRST_ONGOING;
	const wchar_t *additional_error_context = NULL;		// error logging is somewhat removed from error recognition, so use this awkward way of forwarding user-level information
	while ( fetch_status != FRST_ERROR && fetch_status != FRST_FINISHED_ALL )
	{
		if ( iter == sub_list.cend() )
		{
			// Uh oh, there are more result sets than batched calls, we need to fail out
			fetch_status = FRST_ERROR;
			additional_error_context = L"\tThere were more result sets than batched calls.";
			break;
		}

		fetch_status = FRST_ONGOING;
		while ( fetch_status == FRST_ONGOING )
		{
			fetch_status = statement->Fetch_Results( rows_fetched );
			if ( fetch_status != FRST_ERROR && rows_fetched > 0 )
			{
				( *iter )->On_Fetch_Results( call_context->Get_Result_Rows(), rows_fetched );
			}
		}

		if ( fetch_status == FRST_FINISHED_SET )
		{
			( *iter )->On_Fetch_Results_Finished( call_context->Get_Param_Row( input_row ) );
			++iter;
			++input_row;
		}
		else if ( fetch_status == FRST_FINISHED_ALL )
		{
			if ( !statement->Should_Have_Results() || input_row + 1 == list_size )
			{
				for ( ; iter != sub_list.cend(); ++iter, ++input_row )
				{
					( *iter )->On_Fetch_Results_Finished( call_context->Get_Param_Row( input_row ) );
				}
			}
			else
			{
				/*
					This is the case when we have a batch of N, we expect N result sets, but we get back fewer than that.
					We have no way of knowing which row in the batch failed to return a result set (ODBC doesn't consider this an error), 
					so we fail the whole thing and are forced to execute them one by one in order to find the offender.
					It's true that stored procedures that return results should unconditionally select a result set, even if it's empty, but this allows
					us to detect and flag bad procedures rather than giving incorrect results.

					Note that there is no known way of recognizing (and flagging as an error) the case when there are N calls and N result sets,
					but the result sets are not associated 1-1 with the calls.
				*/
				fetch_status = FRST_ERROR;
				additional_error_context = L"\tThere were fewer result sets than batched calls.";
			}
		}
	}

	if ( fetch_status != FRST_FINISHED_ALL )
	{
		Extract_Batch_Error( call_context, statement, additional_error_context, sub_list, result, first_failed_task );
		return;
	}
}	
