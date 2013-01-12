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

#include "Database/Interfaces/DatabaseConnectionInterface.h"
#include "Database/Interfaces/DatabaseEnvironmentInterface.h"
#include "MessageHandling/ProcessMessageHandler.h"
#include "DatabaseProcessMessages.h"

CDatabaseProcessBase::CDatabaseProcessBase( IDatabaseEnvironment *environment, const std::wstring &connection_string, bool process_task_results_locally, const SProcessProperties &properties ) :
	BASECLASS( properties ),
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

void CDatabaseProcessBase::Handle_Run_Database_Task_Request( EProcessID::Enum /*process_id*/, const shared_ptr< const CRunDatabaseTaskRequest > & /*message*/ )
{
}
