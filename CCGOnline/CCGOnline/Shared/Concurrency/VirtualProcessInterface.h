/**********************************************************************************************************************

	VirtualProcessInterface.h
		A component defining the top-level pure virtual interface to a virtual process.

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

#ifndef VIRTUAL_PROCESS_INTERFACE_H
#define VIRTUAL_PROCESS_INTERFACE_H

class CTaskScheduler;
class IVirtualProcessMessage;

struct SProcessProperties;

namespace EVirtualProcessID
{
	enum Enum;
}

// Pure virtual interface for all virtual processes
class IVirtualProcess
{
	public:
		
		IVirtualProcess( void ) {}
		virtual ~IVirtualProcess() {}

		virtual void Initialize( EVirtualProcessID::Enum id ) = 0;

		virtual const SProcessProperties &Get_Properties( void ) const = 0;
		virtual EVirtualProcessID::Enum Get_ID( void ) const = 0;

		virtual void Send_Virtual_Process_Message( EVirtualProcessID::Enum destination_id, const shared_ptr< const IVirtualProcessMessage > &message ) = 0;
		virtual void Log( const std::wstring &message ) = 0;

		virtual CTaskScheduler *Get_Task_Scheduler( void ) const = 0;

		virtual void Flush_System_Messages( void ) = 0;
};

#endif // VIRTUAL_PROCESS_INTERFACE_H
