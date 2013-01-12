/**********************************************************************************************************************

	DatabaseProcessMessages.h
		A component containing definitions for persistence-related process messages

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

#ifndef DATABASE_PROCESS_MESSAGES_H
#define DATABASE_PROCESS_MESSAGES_H

#include "Concurrency/Messaging/ProcessMessage.h"

class IDatabaseTask;

// Requests the interface to a thread tasks or set of thread tasks
class CRunDatabaseTaskRequest : public IProcessMessage
{
	public:

		typedef IProcessMessage BASECLASS;
		
		CRunDatabaseTaskRequest( IDatabaseTask *task ) :
			Task( task )
		{}

		virtual ~CRunDatabaseTaskRequest() {}

		IDatabaseTask *Get_Task( void ) const { return Task; }

	private:

		IDatabaseTask *Task;
};

class CRunDatabaseTaskResponse : public IProcessMessage
{
	public:

		typedef IProcessMessage BASECLASS;
		
		CRunDatabaseTaskResponse( IDatabaseTask *task, bool success ) :
			Task( task ),
			Success( success )
		{}

		virtual ~CRunDatabaseTaskResponse() {}

		IDatabaseTask *Get_Task( void ) const { return Task; }
		bool Was_Successful( void ) const { return Success; }

	private:

		IDatabaseTask *Task;
		bool Success;
};


#endif // DATABASE_PROCESS_MESSAGES_H