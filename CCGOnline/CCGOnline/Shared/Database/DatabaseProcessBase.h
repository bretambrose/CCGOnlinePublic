/**********************************************************************************************************************

	DatabaseProcessBase.h
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

#ifndef DATABASE_PROCESS_BASE_H
#define DATABASE_PROCESS_BASE_H

#include "Concurrency/ThreadProcessBase.h"

class CRunDatabaseTaskRequest;
class IDatabaseConnection;
class IDatabaseEnvironment;
class IDatabaseTaskBatch;

namespace DatabaseTaskIDType
{
	enum Enum;
}

class CDatabaseProcessBase : public CThreadProcessBase
{
	public:

		typedef CThreadProcessBase BASECLASS;

		// Construction/destruction
		CDatabaseProcessBase( IDatabaseEnvironment *environment, const std::wstring &connection_string, bool process_task_results_locally, const SProcessProperties &properties );
		virtual ~CDatabaseProcessBase();

		// IThreadTask interface
		virtual void Initialize( EProcessID::Enum id );

		// IManagedProcess interface
		virtual void Cleanup( void );

	protected:

		// CProcessBase interface
		virtual void Per_Frame_Logic_End( void );
		virtual void Register_Message_Handlers( void );

		// CThreadProcessBase interface
		virtual uint32 Get_Sleep_Interval_In_Milliseconds( void ) const;

		void Add_Batch( IDatabaseTaskBatch *batch );

	private:

		void Handle_Run_Database_Task_Request( EProcessID::Enum process_id, unique_ptr< const CRunDatabaseTaskRequest > &message );

		DatabaseTaskIDType::Enum Allocate_Task_ID( void );

		typedef stdext::hash_map< Loki::TypeInfo, IDatabaseTaskBatch *, STypeInfoContainerHelper > BatchTableType;
		typedef std::vector< Loki::TypeInfo > BatchOrderingType;

		typedef std::pair< EProcessID::Enum, unique_ptr< const CRunDatabaseTaskRequest > > PendingRequestPairType;
		typedef stdext::hash_map< DatabaseTaskIDType::Enum, PendingRequestPairType > PendingRequestTableType;

		BatchTableType Batches;
		BatchOrderingType BatchOrdering;

		PendingRequestTableType PendingRequests;

		DatabaseTaskIDType::Enum NextID;

		IDatabaseEnvironment *Environment;
		std::wstring ConnectionString;
		bool ProcessTaskResultsLocally;

		IDatabaseConnection *Connection;

};

#endif // DATABASE_PROCESS_BASE_H