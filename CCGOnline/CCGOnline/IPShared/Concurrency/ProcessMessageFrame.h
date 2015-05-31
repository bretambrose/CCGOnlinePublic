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

#ifndef PROCESS_MESSAGE_FRAME_H
#define PROCESS_MESSAGE_FRAME_H

class IProcessMessage;

namespace EProcessID
{
	enum Enum;
}

// A container of thread messages
class CProcessMessageFrame
{
	public:

		typedef std::vector< std::unique_ptr< const IProcessMessage > > MessageFrameContainerType;

		CProcessMessageFrame( EProcessID::Enum process_id );
		CProcessMessageFrame( CProcessMessageFrame &&rhs );

		~CProcessMessageFrame();

		EProcessID::Enum Get_Process_ID( void ) const { return ProcessID; }

		void Add_Message( std::unique_ptr< const IProcessMessage > &message );
		void Add_Message( std::unique_ptr< const IProcessMessage > &&message );

		MessageFrameContainerType::iterator begin( void ) { return Messages.begin(); }
		MessageFrameContainerType::iterator end( void ) { return Messages.end(); }

		MessageFrameContainerType::const_iterator cbegin( void ) const { return Messages.cbegin(); }
		MessageFrameContainerType::const_iterator cend( void ) const { return Messages.cend(); }

	private:

		EProcessID::Enum ProcessID;

		std::vector< std::unique_ptr< const IProcessMessage > > Messages;

};

#endif // PROCESS_MESSAGE_FRAME_H
