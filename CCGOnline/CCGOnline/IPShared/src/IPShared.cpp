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

#include <IPShared/IPSharedGlobal.h>

#include <IPCore/System/Time.h>
#include <IPCore/Process/Process.h>
#include <IPShared/Logging/LogInterface.h>
#include <IPShared/StructuredExceptionHandler.h>
#include <IPShared/Concurrency/ProcessStatics.h>
#include <IPShared/GeneratedCode/RegisterIPSharedEnums.h>
#include <IPShared/SharedXMLSerializerRegistration.h>
#include <IPShared/Serialization/SerializationRegistrar.h>
#include <IPShared/SlashCommands/SlashCommandManager.h>

using namespace IP::Command;
using namespace IP::Debug;
using namespace IP::Execution;
using namespace IP::Logging;
using namespace IP::Serialization;

namespace IP
{
namespace Global
{

void Initialize_IPShared( void )
{
	Assert::Initialize( Assert::DLogFunctionType( CLogInterface::LogAssert ) );
	Exception::StructuredExceptionHandler::Initialize();
	CLogInterface::Initialize_Static( IP::Process::Get_Service_Name(), ELogLevel::LL_LOW );
	CProcessStatics::Initialize();

	Register_IPShared_Enums();
	Register_IPShared_XML_Serializers();

	CSlashCommandManager::Initialize();
}


void Shutdown_IPShared( void )
{
	CSlashCommandManager::Shutdown();
	CSerializationRegistrar::Cleanup();
	CProcessStatics::Shutdown();
	CLogInterface::Shutdown_Static();
	Exception::StructuredExceptionHandler::Shutdown();
	Assert::Shutdown();
}

} // namespace Global
} // namespace IP