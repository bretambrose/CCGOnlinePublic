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

#include <IPDatabase/IPDatabase.h>

#include <IPShared/Concurrency/ThreadProcessBase.h>

namespace IP
{
namespace Db
{

class IDatabaseConnection;
class IDatabaseEnvironment;
class IDatabaseTaskBatch;

enum class EDatabaseTaskIDType;

} // namespace Db

namespace Execution
{
namespace Messaging
{

class CRunDatabaseTaskRequest;

} // namespace Messaging


IPDATABASE_API class CDatabaseProcessBase : public CThreadProcessBase
{
	public:

		using BASECLASS = CThreadProcessBase;

		// Construction/destruction
		CDatabaseProcessBase( IP::Db::IDatabaseEnvironment *environment, const IP::String &connection_string, bool process_task_results_locally, const SProcessProperties &properties );
		virtual ~CDatabaseProcessBase();

		// IThreadTask interface
		virtual void Initialize( EProcessID id );

		// IManagedProcess interface
		virtual void Cleanup( void );

	protected:

		// CProcessBase interface
		virtual void Per_Frame_Logic_End( void );
		virtual void Register_Message_Handlers( void );

		// CThreadProcessBase interface
		virtual uint32_t Get_Sleep_Interval_In_Milliseconds( void ) const;

		void Add_Batch( IP::Db::IDatabaseTaskBatch *batch );

	private:

		void Handle_Run_Database_Task_Request( EProcessID process_id, IP::UniquePtr< const Messaging::CRunDatabaseTaskRequest > &message );

		IP::Db::EDatabaseTaskIDType Allocate_Task_ID( void );

		using BatchTableType = IP::UnorderedMap< Loki::TypeInfo, IP::Db::IDatabaseTaskBatch *, STypeInfoContainerHelper >;
		using BatchOrderingType = IP::Vector< Loki::TypeInfo >;

		using PendingRequestPairType = std::pair< EProcessID, IP::UniquePtr< const Messaging::CRunDatabaseTaskRequest > >;
		using PendingRequestTableType = IP::UnorderedMap< IP::Db::EDatabaseTaskIDType, PendingRequestPairType >;

		BatchTableType Batches;
		BatchOrderingType BatchOrdering;

		PendingRequestTableType PendingRequests;

		IP::Db::EDatabaseTaskIDType NextID;

		IP::Db::IDatabaseEnvironment *Environment;
		IP::String ConnectionString;
		bool ProcessTaskResultsLocally;

		IP::Db::IDatabaseConnection *Connection;

};

} // namespace Execution
} // namespace IP