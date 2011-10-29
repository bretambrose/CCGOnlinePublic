/**********************************************************************************************************************

	[Placeholder for eventual source license]

	LoggingThread.h
		A component defining a thread task that performs logging of information to files split by thread key

	(c) Copyright 2011, Bret Ambrose.  All rights reserved.

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