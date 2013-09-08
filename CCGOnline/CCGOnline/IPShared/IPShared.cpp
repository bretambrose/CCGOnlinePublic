/**********************************************************************************************************************

	Shared.cpp
		Component containing initialization and shutdown entry points for the Shared library

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

#include "stdafx.h"

#include "IPShared.h"

#include "Logging/LogInterface.h"
#include "StructuredExceptionHandler.h"
#include "Concurrency/ProcessStatics.h"
#include "IPPlatform/PlatformTime.h"
#include "IPPlatform/PlatformProcess.h"
#include "GeneratedCode/RegisterIPSharedEnums.h"
#include "SharedXMLSerializerRegistration.h"
#include "XML/XMLSerializationRegistrar.h"
#include "SlashCommands/SlashCommandManager.h"

/**********************************************************************************************************************
	NIPShared::Initialize -- Initializes all the static process-wide systems in the Shared library

**********************************************************************************************************************/
void NIPShared::Initialize( void )
{
	CAssertSystem::Initialize( DLogFunctionType( CLogInterface::Log ) );
	CStructuredExceptionHandler::Initialize();
	CLogInterface::Initialize_Static( NPlatform::Get_Service_Name(), LL_LOW );
	CProcessStatics::Initialize();
	CPlatformTime::Initialize();	// does not have a corresponding shutdown function

	Register_IPShared_Enums();
	Register_Shared_XML_Serializers();

	CSlashCommandManager::Initialize();
}

/**********************************************************************************************************************
	NIPShared::Shutdown -- Shuts down and cleans up all the static process-wide systems in the Shared library

**********************************************************************************************************************/
void NIPShared::Shutdown( void )
{
	CSlashCommandManager::Shutdown();
	CXMLSerializationRegistrar::Shutdown();
	CProcessStatics::Shutdown();
	CLogInterface::Shutdown_Static();
	CStructuredExceptionHandler::Shutdown();
	CAssertSystem::Shutdown();
}