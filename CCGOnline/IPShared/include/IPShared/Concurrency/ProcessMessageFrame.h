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

#include <IPCore/Memory/Stl/Vector.h>

namespace IP
{
namespace Execution
{
namespace Messaging
{

class IProcessMessage;

} // namespace Messaging

enum class EProcessID;

// A container of thread messages
IPSHARED_API class CProcessMessageFrame
{
	public:

		using MessageFrameContainerType = IP::Vector< IP::UniquePtr< const Messaging::IProcessMessage > >;

		CProcessMessageFrame( EProcessID process_id );
		CProcessMessageFrame( CProcessMessageFrame &&rhs );

		~CProcessMessageFrame();

		EProcessID Get_Process_ID( void ) const { return ProcessID; }

		void Add_Message( IP::UniquePtr< const Messaging::IProcessMessage > &message );
		void Add_Message( IP::UniquePtr< const Messaging::IProcessMessage > &&message );

		MessageFrameContainerType::iterator begin( void ) { return Messages.begin(); }
		MessageFrameContainerType::iterator end( void ) { return Messages.end(); }

		MessageFrameContainerType::const_iterator cbegin( void ) const { return Messages.cbegin(); }
		MessageFrameContainerType::const_iterator cend( void ) const { return Messages.cend(); }

	private:

		EProcessID ProcessID;

		IP::Vector< IP::UniquePtr< const Messaging::IProcessMessage > > Messages;

};

} // namespace Execution
} // namespace IP
