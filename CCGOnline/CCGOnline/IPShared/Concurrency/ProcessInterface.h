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

namespace IP
{
namespace Execution
{
namespace Messaging
{

class IProcessMessage;

} // namespace Messaging

class CTaskScheduler;
struct SProcessProperties;

enum class EProcessID;

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

		virtual void Initialize( EProcessID id ) = 0;

		virtual const SProcessProperties &Get_Properties( void ) const = 0;
		virtual EProcessID Get_ID( void ) const = 0;
		virtual EProcessExecutionMode Get_Execution_Mode( void ) const = 0;

		virtual void Send_Process_Message( EProcessID destination_id, std::unique_ptr< const Messaging::IProcessMessage > &message ) = 0;
		virtual void Send_Process_Message( EProcessID destination_id, std::unique_ptr< const Messaging::IProcessMessage > &&message ) = 0;
		virtual void Send_Manager_Message( std::unique_ptr< const Messaging::IProcessMessage > &message ) = 0;
		virtual void Send_Manager_Message( std::unique_ptr< const Messaging::IProcessMessage > &&message ) = 0;
		virtual void Log( std::wstring &&message ) = 0;

		virtual CTaskScheduler *Get_Task_Scheduler( void ) const = 0;

		virtual void Flush_System_Messages( void ) = 0;
};

} // namespace Execution
} // namespace IP
