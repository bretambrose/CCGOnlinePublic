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

#include <IPShared/Concurrency/ProcessMessageFrame.h>

#include <IPShared/Concurrency/Messaging/ProcessMessage.h>

namespace IP
{
namespace Execution
{

CProcessMessageFrame::CProcessMessageFrame( EProcessID process_id ) :
	ProcessID( process_id ),
	Messages()
{
}


CProcessMessageFrame::CProcessMessageFrame( CProcessMessageFrame &&rhs ) :
	ProcessID( rhs.ProcessID ),
	Messages( std::move( rhs.Messages ) )
{
}


CProcessMessageFrame::~CProcessMessageFrame()
{
}


void CProcessMessageFrame::Add_Message( IP::UniquePtr< const Messaging::IProcessMessage > &message )
{
	Messages.emplace_back( std::move( message ) );
}


void CProcessMessageFrame::Add_Message( IP::UniquePtr< const Messaging::IProcessMessage > &&message )
{
	Messages.emplace_back( std::move( message ) );
}

} // namespace Execution
} // namespace IP



