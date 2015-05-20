/**********************************************************************************************************************

	DatabaseProcessBase.cpp
		A component defining ??

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

#include "stdafx.h"

#include "DatabaseProcessBase.h"

#include "Interfaces/DatabaseTaskBatchInterface.h"
#include "Interfaces/DatabaseConnectionInterface.h"
#include "Interfaces/DatabaseEnvironmentInterface.h"
#include "Interfaces/DatabaseTaskInterface.h"
#include "IPShared/MessageHandling/ProcessMessageHandler.h"
#include "DatabaseProcessMessages.h"

CDatabaseProcessBase::CDatabaseProcessBase( IDatabaseEnvironment *environment, const std::wstring &connection_string, bool process_task_results_locally, const SProcessProperties &properties ) :
	BASECLASS( properties ),
	Batches(),
	BatchOrdering(),
	PendingRequests(),
	NextID( static_cast< DatabaseTaskIDType::Enum >( DatabaseTaskIDType::INVALID + 1 ) ),
	Environment( environment ),
	ConnectionString( connection_string ),
	ProcessTaskResultsLocally( process_task_results_locally ),
	Connection( NULL )
{
	FATAL_ASSERT( Environment != nullptr );
}

CDatabaseProcessBase::~CDatabaseProcessBase()
{
	FATAL_ASSERT( Connection == nullptr );
	FATAL_ASSERT( Environment == nullptr );
}	

void CDatabaseProcessBase::Initialize( EProcessID::Enum id )
{
	BASECLASS::Initialize( id );

	Connection = Environment->Add_Connection( ConnectionString.c_str(), true );
	FATAL_ASSERT( Connection != nullptr );
}

void CDatabaseProcessBase::Cleanup( void )
{
	FATAL_ASSERT( Environment != nullptr );

	BASECLASS::Cleanup();

	PendingRequests.clear();

	for ( auto iter = Batches.cbegin(); iter != Batches.end(); ++iter )
	{
		delete iter->second;
	}

	Batches.clear();
	BatchOrdering.clear();

	if ( Connection != nullptr )
	{
		Environment->Shutdown_Connection( Connection );
		delete Connection;
		Connection = nullptr;
	}

	Environment = nullptr;
}

void CDatabaseProcessBase::Per_Frame_Logic_End( void )
{
	FATAL_ASSERT( Environment != nullptr );
	FATAL_ASSERT( Connection != nullptr );

	BASECLASS::Per_Frame_Logic_End();

	DBTaskBaseListType successes;
	DBTaskBaseListType failures;

	for ( auto iter = BatchOrdering.cbegin(), end = BatchOrdering.cend(); iter != end; ++iter )
	{
		auto batch_task_iter = Batches.find( *iter );
		FATAL_ASSERT( batch_task_iter != Batches.end() );

		IDatabaseTaskBatch *batch = batch_task_iter->second;
		batch->Execute_Tasks( Connection, successes, failures );
	}

	for ( auto success_iter = successes.cbegin(), end = successes.cend(); success_iter != end; ++success_iter )
	{
		IDatabaseTaskBase *task = *success_iter;
		auto pending_request_iter = PendingRequests.find( task->Get_ID() );
		FATAL_ASSERT( pending_request_iter != PendingRequests.end() );

		if ( ProcessTaskResultsLocally )
		{
			( *success_iter )->On_Task_Success();
		}
		else
		{
			unique_ptr< const IProcessMessage > response( new CRunDatabaseTaskResponse( pending_request_iter->second.second, true ) );
			Send_Process_Message( pending_request_iter->second.first, response );
		}

		PendingRequests.erase( pending_request_iter );
	}

	for ( auto failure_iter = failures.cbegin(), end = failures.cend(); failure_iter != end; ++failure_iter )
	{
		IDatabaseTaskBase *task = *failure_iter;
		auto pending_request_iter = PendingRequests.find( task->Get_ID() );
		FATAL_ASSERT( pending_request_iter != PendingRequests.end() );

		if ( ProcessTaskResultsLocally )
		{
			( *failure_iter )->On_Task_Failure();
		}
		else
		{
			unique_ptr< const IProcessMessage > response( new CRunDatabaseTaskResponse( pending_request_iter->second.second, false ) );
			Send_Process_Message( pending_request_iter->second.first, response );
		}

		PendingRequests.erase( pending_request_iter );
	}
}

void CDatabaseProcessBase::Register_Message_Handlers( void )
{
	BASECLASS::Register_Message_Handlers();

	REGISTER_THIS_HANDLER( CRunDatabaseTaskRequest, CDatabaseProcessBase, Handle_Run_Database_Task_Request )
}

uint32 CDatabaseProcessBase::Get_Sleep_Interval_In_Milliseconds( void ) const
{
	return 1;
}

void CDatabaseProcessBase::Add_Batch( IDatabaseTaskBatch *batch )
{
	FATAL_ASSERT( batch != nullptr );

	Loki::TypeInfo type_info = batch->Get_Task_Type_Info();
	FATAL_ASSERT( Batches.find( type_info ) == Batches.end() );

	Batches.insert( BatchTableType::value_type( type_info, batch ) );
	BatchOrdering.push_back( type_info );
}

void CDatabaseProcessBase::Handle_Run_Database_Task_Request( EProcessID::Enum process_id, unique_ptr< const CRunDatabaseTaskRequest > &message )
{
	IDatabaseTask *task = message->Get_Task();

	Loki::TypeInfo hash_key( typeid( *task ) );
	auto iter = Batches.find( hash_key );
	FATAL_ASSERT( iter != Batches.end() );

	DatabaseTaskIDType::Enum task_id = Allocate_Task_ID();
	task->Set_ID( task_id );
	iter->second->Add_Task( task );

	PendingRequests.insert( PendingRequestTableType::value_type( task_id, PendingRequestPairType( process_id, std::move( message ) ) ) );
}

DatabaseTaskIDType::Enum CDatabaseProcessBase::Allocate_Task_ID( void )
{
	DatabaseTaskIDType::Enum id = NextID;
	NextID = static_cast< DatabaseTaskIDType::Enum >( NextID + 1 );

	return id;
}
