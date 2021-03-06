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

#include "ProcessMessage.h"

#include "IPPlatform/PlatformTime.h"
#include "IPShared/Concurrency/ProcessProperties.h"

namespace IP
{
namespace Execution
{

enum class EProcessID;

namespace Messaging
{

// A message asking the logging thread to write some information to a file
class CLogRequestMessage : public IProcessMessage
{
	public:

		using BASECLASS = IProcessMessage;

		CLogRequestMessage( const IP::Execution::SProcessProperties &source_properties, std::wstring &&message );
		CLogRequestMessage( const IP::Execution::SProcessProperties &source_properties, const std::wstring &message );
		virtual ~CLogRequestMessage() = default;

		const IP::Execution::SProcessProperties &Get_Source_Properties( void ) const { return SourceProperties; }
		const std::wstring &Get_Message( void ) const { return Message; }
		IP::Time::SystemTimePoint Get_Time( void ) const { return Time; }

	private:

		IP::Execution::SProcessProperties SourceProperties;
		std::wstring Message;
		IP::Time::SystemTimePoint Time;
};
 
} // namespace Messaging
} // namespace Execution
} // namespace IP

