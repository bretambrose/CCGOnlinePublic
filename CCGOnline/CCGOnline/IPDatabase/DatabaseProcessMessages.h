/**********************************************************************************************************************

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

#pragma once

#include "IPShared/Concurrency/Messaging/ProcessMessage.h"

namespace IP
{
namespace Db
{

class IDatabaseTask;

} // namespace Db

namespace Execution
{
namespace Messaging
{

// Requests the interface to a thread tasks or set of thread tasks
class CRunDatabaseTaskRequest : public IProcessMessage
{
	public:

		using BASECLASS = IProcessMessage;
		
		CRunDatabaseTaskRequest( IP::Db::IDatabaseTask *task ) :
			Task( task )
		{}

		virtual ~CRunDatabaseTaskRequest();

		IP::Db::IDatabaseTask *Get_Task( void ) const { return Task.get(); }

	private:

		std::unique_ptr< IP::Db::IDatabaseTask > Task;
};

class CRunDatabaseTaskResponse : public IProcessMessage
{
	public:

		using BASECLASS = IProcessMessage;
		
		CRunDatabaseTaskResponse( std::unique_ptr< const CRunDatabaseTaskRequest > &request, bool success ) :
			Request( std::move( request ) ),
			Success( success )
		{}

		virtual ~CRunDatabaseTaskResponse();

		const std::unique_ptr< const CRunDatabaseTaskRequest > &Get_Request( void ) const { return Request; }
		bool Was_Successful( void ) const { return Success; }

	private:

		std::unique_ptr< const CRunDatabaseTaskRequest > Request;
		bool Success;
};

} // namespace Messaging
} // namespace Execution
} // namespace IP