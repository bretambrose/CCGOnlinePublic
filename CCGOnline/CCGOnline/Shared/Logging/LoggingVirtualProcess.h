/**********************************************************************************************************************

	LoggingVirtualProcess.h
		A component defining a virtual process that performs logging of information to files split by thread key

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

#ifndef LOGGING_VIRTUAL_PROCESS_H
#define LOGGING_VIRTUAL_PROCESS_H

#include "Concurrency/VirtualProcessBase.h"

class CLogRequestMessage;
class CLogFile;

// A class that performs logging of information to files split by thread key
class CLoggingVirtualProcess : public CVirtualProcessBase
{
	public:

		typedef CVirtualProcessBase BASECLASS;

		// Construction/destruction
		CLoggingVirtualProcess( void );
		virtual ~CLoggingVirtualProcess();

		// CThreadTaskBase public interface
		virtual void Initialize( void );

		virtual void Log( const std::wstring &message );

		virtual ETimeType Get_Time_Type( void ) const;
		virtual bool Is_Root_Thread( void ) const { return true; }

		virtual void Service( double elapsed_seconds, const CVirtualProcessExecutionContext &context );

	protected:

		// CVirtualProcessBase protected interface
		virtual void Register_Message_Handlers( void );

	private:

		void Shutdown( void );

		shared_ptr< CLogFile > Get_Log_File( EThreadSubject subject ) const;

		std::wstring Build_File_Name( EThreadSubject subject ) const;
		std::wstring Build_Log_Message( const SThreadKey &source_key, const std::wstring &message, uint64 system_time ) const;

		virtual void Handle_Shutdown_Self_Request( const SThreadKey &key, const shared_ptr< const CShutdownSelfRequest > &message );
		void Handle_Log_Request_Message( const SThreadKey &key, const shared_ptr< const CLogRequestMessage > &message );

		void Handle_Log_Request_Message_Aux( const SThreadKey &key, const std::wstring &message, uint64 system_time );

		// Private Data
		stdext::hash_map< EThreadSubject, shared_ptr< CLogFile > > LogFiles;

		uint32 PID;

		bool IsShuttingDown;
};

#endif // LOGGING_VIRTUAL_PROCESS_H