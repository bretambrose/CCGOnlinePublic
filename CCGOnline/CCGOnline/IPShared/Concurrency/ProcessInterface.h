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

#ifndef PROCESS_INTERFACE_H
#define PROCESS_INTERFACE_H

class CTaskScheduler;
class IProcessMessage;

struct SProcessProperties;

namespace EProcessID
{
	enum Enum;
}

enum class EProcessExecutionMode
{
	TBB_TASK,
	THREAD
};

// Pure virtual interface for all virtual processes
class IProcess
{
	public:
		
		IProcess( void ) {}
		virtual ~IProcess() = default;

		virtual void Initialize( EProcessID::Enum id ) = 0;

		virtual const SProcessProperties &Get_Properties( void ) const = 0;
		virtual EProcessID::Enum Get_ID( void ) const = 0;
		virtual EProcessExecutionMode Get_Execution_Mode( void ) const = 0;

		virtual void Send_Process_Message( EProcessID::Enum destination_id, std::unique_ptr< const IProcessMessage > &message ) = 0;
		virtual void Send_Process_Message( EProcessID::Enum destination_id, std::unique_ptr< const IProcessMessage > &&message ) = 0;
		virtual void Send_Manager_Message( std::unique_ptr< const IProcessMessage > &message ) = 0;
		virtual void Send_Manager_Message( std::unique_ptr< const IProcessMessage > &&message ) = 0;
		virtual void Log( std::wstring &&message ) = 0;

		virtual CTaskScheduler *Get_Task_Scheduler( void ) const = 0;

		virtual void Flush_System_Messages( void ) = 0;
};

#endif // PROCESS_INTERFACE_H
