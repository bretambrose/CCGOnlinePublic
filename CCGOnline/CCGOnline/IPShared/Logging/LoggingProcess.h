/**********************************************************************************************************************

	LoggingProcess.h
		A component defining a process that performs logging of information to files split by thread key

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

#ifndef LOGGING_PROCESS_H
#define LOGGING_PROCESS_H

#include "IPShared/Concurrency/TaskProcessBase.h"

class CLogRequestMessage;
class CLogFile;

namespace EProcessSubject
{
	enum Enum;
}

// A class that performs logging of information to files split by thread key
class CLoggingProcess : public CTaskProcessBase
{
	public:

		typedef CTaskProcessBase BASECLASS;

		// Construction/destruction
		CLoggingProcess( const SProcessProperties &properties );
		virtual ~CLoggingProcess();

		// CThreadTaskBase public interface
		virtual void Initialize( EProcessID::Enum id ) override;

		virtual void Log( std::wstring &&message ) override;

		virtual ETimeType Get_Time_Type( void ) const override;
		virtual bool Is_Root_Thread( void ) const override { return true; }

		virtual void Run( const CProcessExecutionContext &context ) override;

	protected:

		// CProcessBase protected interface
		virtual void Register_Message_Handlers( void ) override;

		virtual void On_Shutdown_Self_Request( void ) override;

	private:

		void Shutdown( void );

		CLogFile *Get_Log_File( EProcessSubject::Enum subject ) const;

		std::wstring Build_File_Name( EProcessSubject::Enum subject ) const;
		std::wstring Build_Log_Message( EProcessID::Enum process_id, const SProcessProperties &source_properties, const std::wstring &message, uint64_t system_time ) const;

		void Handle_Log_Request_Message( EProcessID::Enum source_process_id, unique_ptr< const CLogRequestMessage > &message );

		void Handle_Log_Request_Message_Aux( EProcessID::Enum source_process_id, const SProcessProperties &properties, const std::wstring &message, uint64_t system_time );

		// Private Data
		typedef std::unordered_map< EProcessSubject::Enum, unique_ptr< CLogFile > > LogFileTableType; 
		LogFileTableType LogFiles;

		uint32_t PID;

		bool IsShuttingDown;
};

#endif // LOGGING_PROCESS_H