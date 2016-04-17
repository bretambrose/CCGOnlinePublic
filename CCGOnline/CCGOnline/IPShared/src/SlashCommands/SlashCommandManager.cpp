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

#include <IPShared/SlashCommands/SlashCommandManager.h>

#include <IPCore/Utils/StringUtils.h>
#include <IPShared/SlashCommands/SlashCommandDefinition.h>
#include <IPShared/SlashCommands/SlashCommandDataDefinition.h>
#include <IPShared/SlashCommands/SlashCommandInstance.h>
#include <IPShared/Serialization/XML/XMLLoadableTable.h>

#include <regex>

using namespace IP::Serialization;
using namespace IP::Serialization::XML;

namespace IP
{
namespace Command
{

IP::UniquePtr< CXMLLoadableTable< IP::String, CSlashCommandDataDefinition > > CSlashCommandManager::DataDefinitions( nullptr );
IP::UnorderedMap< IP::String, const CSlashCommandDefinition * > CSlashCommandManager::Definitions;
IP::UnorderedMap< IP::String, CSlashCommandManager::CommandHandlerDelegate > CSlashCommandManager::CommandHandlers;

void CSlashCommandManager::Initialize( void )
{
	Shutdown();
	DataDefinitions = IP::Make_Unique< CXMLLoadableTable< IP::String, CSlashCommandDataDefinition > >( MEMORY_TAG, &CSlashCommandDataDefinition::Get_Key, "Commands" );
	DataDefinitions->Set_Post_Load_Function( &CSlashCommandDataDefinition::Post_Load_XML );
}


void CSlashCommandManager::Shutdown( void )
{
	for ( auto iter = Definitions.cbegin(), end = Definitions.cend(); iter != end; ++iter )
	{
		IP::Delete( iter->second );
	}

	Definitions.clear();

	DataDefinitions = nullptr;

	CommandHandlers.clear();
}


void CSlashCommandManager::Load_Command_File( const IP::String &file_name )
{
	DataDefinitions->Load( file_name );

	// Create command wrappers for all loaded commands
	for ( auto iter = DataDefinitions->cbegin(), end = DataDefinitions->cend(); iter != end; ++iter )
	{
		if ( Definitions.find( iter->first ) != Definitions.cend() )
		{
			continue;
		}

		CSlashCommandDefinition *definition = IP::New< CSlashCommandDefinition >( MEMORY_TAG, iter->second );
		Definitions[ iter->first ] = definition;
	}

	// Create command family placeholder for command families
	for ( auto iter = DataDefinitions->cbegin(), end = DataDefinitions->cend(); iter != end; ++iter )
	{
		const CSlashCommandDataDefinition *data_definition = iter->second;

		if ( data_definition->Get_Sub_Command().size() == 0 )
		{
			continue;
		}

		IP::String upper_command;
		IP::StringUtils::To_Upper_Case( data_definition->Get_Command(), upper_command );

		if ( Definitions.find( upper_command ) == Definitions.cend() )
		{
			CSlashCommandDefinition *family_data_definition = IP::New< CSlashCommandDefinition >( MEMORY_TAG, nullptr );
			Definitions[ upper_command ] = family_data_definition;
		}
	}
}


IP::String CSlashCommandManager::Concat_Command( const IP::String &command, const IP::String &sub_command )
{
	if ( sub_command.size() == 0 )
	{
		return command;
	}
	else
	{
		return command + "+" + sub_command;
	}
}

static std::regex _CommandPattern( "^/(\\w+).*" );	// extracts the command
static std::regex _SubCommandPattern( "^/(\\w+)\\s+(\\w+).*" ); // extracts the subcommand, if needed


bool CSlashCommandManager::Parse_Command( const IP::String &command_line, CSlashCommandInstance &command_instance, IP::String &error_msg )
{
	// extract the command
	std::cmatch command_match_results;
	std::regex_search( command_line.c_str(), command_match_results, _CommandPattern );

	IP::String command = command_match_results[ 1 ].str().c_str();
	if ( command.size() == 0 )
	{
		error_msg = "Invalid slash command specification";
		return false;
	}

	IP::String upper_command;
	IP::StringUtils::To_Upper_Case( command, upper_command );
	auto iter = Definitions.find( upper_command );
	if ( iter == Definitions.cend() )
	{
		error_msg = "No such command exists: " + command;
		return false;
	}

	// if there's no subcommand, extract parameters
	const CSlashCommandDefinition *definition = iter->second;
	if ( !definition->Is_Family() )
	{
		return command_instance.Parse( command_line, definition, error_msg );
	}

	// extract the subcommand
	std::cmatch sub_command_match_results;
	std::regex_search( command_line.c_str(), sub_command_match_results, _SubCommandPattern );

	IP::String sub_command = sub_command_match_results[ 2 ].str().c_str();
	if ( sub_command.size() == 0 )
	{
		error_msg = "Invalid subcommand for command family: " + command;
		return false;
	}

	IP::String upper_sub_command;
	IP::StringUtils::To_Upper_Case( sub_command, upper_sub_command );

	IP::String concat_command = Concat_Command( upper_command, upper_sub_command );
	iter = Definitions.find( concat_command );
	if ( iter == Definitions.end() )
	{
		error_msg = "Unknown subcommand ( " + sub_command + " ) for command family: " + command;
		return false;
	}

	// extract parameters
	return command_instance.Parse( command_line, iter->second, error_msg );
}


bool CSlashCommandManager::Handle_Command( const IP::String &command_line, IP::String &error_msg )
{
	CSlashCommandInstance instance;
	if ( !Parse_Command( command_line, instance, error_msg ) )
	{
		return false;
	}

	return Handle_Command( instance, error_msg );
}


bool CSlashCommandManager::Handle_Command( const CSlashCommandInstance &command, IP::String &error_msg )
{
	IP::String concat_command = Concat_Command( command.Get_Command(), command.Get_Sub_Command() );

	IP::String upper_concat_command;
	IP::StringUtils::To_Upper_Case( concat_command, upper_concat_command );
	
	auto iter = CommandHandlers.find( upper_concat_command );
	if ( iter == CommandHandlers.end() )
	{
		error_msg = "Command does not exist";
		return false;
	}

	return iter->second( command, error_msg );
}


void CSlashCommandManager::Register_Command_Handler( const IP::String &command, CommandHandlerDelegate handler )
{
	IP::String upper_command;
	IP::StringUtils::To_Upper_Case( command, upper_command );

	FATAL_ASSERT( CommandHandlers.find( upper_command ) == CommandHandlers.cend() );
	CommandHandlers[ upper_command ] = handler;
}


void CSlashCommandManager::Register_Command_Handler( const IP::String &command, const IP::String &sub_command, CommandHandlerDelegate handler )
{
	IP::String concat_command = Concat_Command( command, sub_command );

	IP::String upper_concat_command;
	IP::StringUtils::To_Upper_Case( concat_command, upper_concat_command );

	FATAL_ASSERT( CommandHandlers.find( upper_concat_command ) == CommandHandlers.cend() );
	CommandHandlers[ upper_concat_command ] = handler;
}

} // namespace Command
} // namespace IP