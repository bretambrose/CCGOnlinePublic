/**********************************************************************************************************************

	SlashCommandManager.h
		A component that tracks all slash commands for the process and is the entry point for parsing and handling
		them.

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

#ifndef SLASH_COMMAND_MANAGER_H
#define SLASH_COMMAND_MANAGER_H

class CSlashCommandInstance;
class CSlashCommandDefinition;
class CSlashCommandDataDefinition;
template< typename K, typename T > class CXMLLoadableTable;

// Static interface to the slash command system
class CSlashCommandManager
{
	public:

		typedef fastdelegate::FastDelegate2< const CSlashCommandInstance &, std::wstring &, bool > CommandHandlerDelegate;

		// Init/Cleanup
		static void Initialize( void );
		static void Shutdown( void );

		static void Register_Command_Handler( const std::wstring &command, CommandHandlerDelegate handler );
		static void Register_Command_Handler( const std::wstring &command, const std::wstring &sub_command, CommandHandlerDelegate handler );

		static void Load_Command_File( const std::string &file_name );

		// Operations
		static bool Parse_Command( const std::wstring &command_line, CSlashCommandInstance &command_instance, std::wstring &error_msg );

		static bool Handle_Command( const std::wstring &command_line, std::wstring &error_msg );
		static bool Handle_Command( const CSlashCommandInstance &command, std::wstring &error_msg );

		// Util
		static std::wstring Concat_Command( const std::wstring &command, const std::wstring &sub_command );

	private:

		// Data
		static CXMLLoadableTable< std::wstring, CSlashCommandDataDefinition > *DataDefinitions;

		static std::unordered_map< std::wstring, const CSlashCommandDefinition * > Definitions;

		static std::unordered_map< std::wstring, CommandHandlerDelegate > CommandHandlers;
};

#endif // SLASH_COMMAND_MANAGER_H