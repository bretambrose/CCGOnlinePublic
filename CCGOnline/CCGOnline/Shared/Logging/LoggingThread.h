/**********************************************************************************************************************

	LoggingThread.h
		A component defining a thread task that performs logging of information to files split by thread key

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

#ifndef LOGGING_THREAD_H
#define LOGGING_THREAD_H

#include "Concurrency/ThreadTaskBase.h"

class CLogRequestMessage;
class CLogFile;

// A class that performs logging of information to files split by thread key
class CLoggingThreadTask : public CThreadTaskBase
{
	public:

		typedef CThreadTaskBase BASECLASS;

		// Construction/destruction
		CLoggingThreadTask( void );
		virtual ~CLoggingThreadTask();

		// CThreadTaskBase public interface
		virtual void Initialize( void );

		virtual void Log( const std::wstring &message );

		virtual ETimeType Get_Time_Type( void ) const;
		virtual bool Is_Root_Thread( void ) const { return true; }

		virtual void Service( double elapsed_seconds, const CThreadTaskExecutionContext &context );

	protected:

		// CThreadTaskBase protected interface
		virtual void Register_Message_Handlers( void );

	private:

		void Shutdown( void );

		shared_ptr< CLogFile > Get_Log_File( EThreadSubject subject ) const;

		std::wstring Build_File_Name( EThreadSubject subject ) const;
		std::wstring Build_Log_Message( const SThreadKey &source_key, const std::wstring &message, uint64 system_time ) const;

		virtual void Handle_Shutdown_Thread_Request( const SThreadKey &key, const shared_ptr< const CShutdownThreadRequest > &message );
		void Handle_Log_Request_Message( const SThreadKey &key, const shared_ptr< const CLogRequestMessage > &message );

		void Handle_Log_Request_Message_Aux( const SThreadKey &key, const std::wstring &message, uint64 system_time );

		// Private Data
		stdext::hash_map< EThreadSubject, shared_ptr< CLogFile > > LogFiles;

		uint32 PID;

		bool IsShuttingDown;
};

#endif // LOGGING_THREAD_H