/**********************************************************************************************************************

	SlashCommandManager.cpp
		A component that tracks all slash commands for the process

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

#include "SlashCommandManager.h"

#include "SlashCommandDefinition.h"
#include "SlashCommandDataDefinition.h"
#include "SlashCommandInstance.h"
#include "XML/XMLLoadableTable.h"
#include "StringUtils.h"
#include <regex>

CXMLLoadableTable< std::wstring, CSlashCommandDataDefinition > *CSlashCommandManager::DataDefinitions( nullptr );
stdext::hash_map< std::wstring, const CSlashCommandDefinition * > CSlashCommandManager::Definitions;

void CSlashCommandManager::Initialize( void )
{
	Shutdown();
	DataDefinitions = new CXMLLoadableTable< std::wstring, CSlashCommandDataDefinition >( &CSlashCommandDataDefinition::Get_Key, L"Commands" );
	DataDefinitions->Set_Post_Load_Function( &CSlashCommandDataDefinition::Post_Load_XML );
}

void CSlashCommandManager::Shutdown( void )
{
	for ( auto iter = Definitions.cbegin(); iter != Definitions.cend(); ++iter )
	{
		delete iter->second;
	}

	Definitions.clear();

	if ( DataDefinitions != nullptr )
	{
		delete DataDefinitions;
		DataDefinitions = nullptr;
	}
}

void CSlashCommandManager::Load_Command_File( const std::string &file_name )
{
	DataDefinitions->Load( file_name );

	for ( auto iter = DataDefinitions->cbegin(); iter != DataDefinitions->cend(); ++iter )
	{
		if ( Definitions.find( iter->first ) != Definitions.end() )
		{
			continue;
		}

		CSlashCommandDefinition *definition = new CSlashCommandDefinition( iter->second );
		Definitions[ iter->first ] = definition;
	}

	for ( auto iter = DataDefinitions->cbegin(); iter != DataDefinitions->cend(); ++iter )
	{
		const CSlashCommandDataDefinition *data_definition = iter->second;

		if ( data_definition->Get_Sub_Command().size() == 0 )
		{
			continue;
		}

		std::wstring upper_command;
		NStringUtils::To_Upper_Case( data_definition->Get_Command(), upper_command );

		if ( Definitions.find( upper_command ) == Definitions.end() )
		{
			CSlashCommandDefinition *family_data_definition = new CSlashCommandDefinition( nullptr );
			Definitions[ upper_command ] = family_data_definition;
		}
	}
}

std::wstring CSlashCommandManager::Concat_Command( const std::wstring &command, const std::wstring &sub_command )
{
	if ( sub_command.size() == 0 )
	{
		return command;
	}
	else
	{
		return command + L"+" + sub_command;
	}
}

static std::tr1::wregex _CommandPattern( L"^/(\\w+).*" );
static std::tr1::wregex _SubCommandPattern( L"^/(\\w+)\\s+(\\w+).*" );

bool CSlashCommandManager::Parse_Command( const std::wstring &command_line, CSlashCommandInstance &command_instance, std::wstring &error_msg )
{
	std::tr1::wcmatch command_match_results;
	std::tr1::regex_search( command_line.c_str(), command_match_results, _CommandPattern );

	std::wstring command = command_match_results[ 1 ];
	if ( command.size() == 0 )
	{
		error_msg = L"Invalid slash command specification";
		return false;
	}

	std::wstring upper_command;
	NStringUtils::To_Upper_Case( command, upper_command );
	auto iter = Definitions.find( upper_command );
	if ( iter == Definitions.end() )
	{
		error_msg = L"No such command exists: " + command;
		return false;
	}

	const CSlashCommandDefinition *definition = iter->second;
	if ( !definition->Is_Family() )
	{
		return command_instance.Parse( command_line, definition, error_msg );
	}

	std::tr1::wcmatch sub_command_match_results;
	std::tr1::regex_search( command_line.c_str(), sub_command_match_results, _SubCommandPattern );

	std::wstring sub_command = sub_command_match_results[ 2 ];
	if ( sub_command.size() == 0 )
	{
		error_msg = L"Invalid subcommand for command family: " + command;
		return false;
	}

	std::wstring upper_sub_command;
	NStringUtils::To_Upper_Case( sub_command, upper_sub_command );

	std::wstring concat_command = Concat_Command( upper_command, upper_sub_command );
	iter = Definitions.find( concat_command );
	if ( iter == Definitions.end() )
	{
		error_msg = L"Unknown subcommand ( " + sub_command + L" ) for command family: " + command;
		return false;
	}

	return command_instance.Parse( command_line, iter->second, error_msg );
}

bool CSlashCommandManager::Handle_Command( const CSlashCommandInstance & /*command*/, std::wstring & /*error_msg*/ )
{
	return false;
}
