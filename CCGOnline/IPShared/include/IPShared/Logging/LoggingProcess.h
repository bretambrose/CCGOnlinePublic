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

#include <IPShared/IPShared.h>

#include <IPCore/Memory/Stl/UnorderedMap.h>
#include <IPCore/System/Time.h>
#include <IPShared/Concurrency/TaskProcessBase.h>

namespace EProcessSubject
{
	enum Enum;
}

namespace IP
{
namespace Execution
{
namespace Messaging
{

class CLogRequestMessage;

} // namespace Messaging

class CLogFile;

// A class that performs logging of information to files split by thread key
IPSHARED_API class CLoggingProcess : public CTaskProcessBase
{
	public:

		using BASECLASS = CTaskProcessBase;

		// Construction/destruction
		CLoggingProcess( const SProcessProperties &properties );
		virtual ~CLoggingProcess();

		// CThreadTaskBase public interface
		virtual void Initialize( EProcessID id ) override;

		virtual void Log( IP::String &&message ) override;

		virtual bool Is_Root_Thread( void ) const override { return true; }

		virtual void Run( const CProcessExecutionContext &context ) override;

	protected:

		// CProcessBase protected interface
		virtual void Register_Message_Handlers( void ) override;

		virtual void On_Shutdown_Self_Request( void ) override;

	private:

		void Shutdown( void );

		CLogFile *Get_Log_File( EProcessSubject::Enum subject ) const;

		IP::String Build_File_Name( EProcessSubject::Enum subject ) const;
		IP::String Build_Log_Message( EProcessID process_id, const SProcessProperties &source_properties, const IP::String &message, IP::Time::SystemTimePoint system_time ) const;

		void Handle_Log_Request_Message( EProcessID source_process_id, IP::UniquePtr< const Messaging::CLogRequestMessage > &message );

		void Handle_Log_Request_Message_Aux( EProcessID source_process_id, const SProcessProperties &properties, const IP::String &message, IP::Time::SystemTimePoint system_time );

		// Private Data
		using LogFileTableType = IP::UnorderedMap< EProcessSubject::Enum, IP::UniquePtr< CLogFile > >; 
		LogFileTableType LogFiles;

		uint32_t PID;

		bool IsShuttingDown;
};

} // namespace Execution
} // namespace IP