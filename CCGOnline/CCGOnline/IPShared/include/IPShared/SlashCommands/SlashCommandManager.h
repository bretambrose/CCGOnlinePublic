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

#include <IPCore/Memory/Memory.h>
#include <IPCore/Memory/Stl/String.h>
#include <IPCore/Memory/Stl/UnorderedMap.h>

#include <functional>

namespace IP
{
namespace Serialization
{
namespace XML
{

template< typename K, typename T > class CXMLLoadableTable;

} // namespace XML
} // namespace Serialization

namespace Command
{

class CSlashCommandInstance;
class CSlashCommandDefinition;
class CSlashCommandDataDefinition;

// Static interface to the slash command system
IPSHARED_API class CSlashCommandManager
{
	public:

		using CommandHandlerDelegate = std::function< bool( const CSlashCommandInstance &, IP::String & ) >;

		// Init/Cleanup
		static void Initialize( void );
		static void Shutdown( void );

		static void Register_Command_Handler( const IP::String &command, CommandHandlerDelegate handler );
		static void Register_Command_Handler( const IP::String &command, const IP::String &sub_command, CommandHandlerDelegate handler );

		static void Load_Command_File( const IP::String &file_name );

		// Operations
		static bool Parse_Command( const IP::String &command_line, CSlashCommandInstance &command_instance, IP::String &error_msg );

		static bool Handle_Command( const IP::String &command_line, IP::String &error_msg );
		static bool Handle_Command( const CSlashCommandInstance &command, IP::String &error_msg );

		// Util
		static IP::String Concat_Command( const IP::String &command, const IP::String &sub_command );

	private:

		// Data
		static IP::UniquePtr< IP::Serialization::XML::CXMLLoadableTable< IP::String, CSlashCommandDataDefinition > > DataDefinitions;

		static IP::UnorderedMap< IP::String, const CSlashCommandDefinition * > Definitions;

		static IP::UnorderedMap< IP::String, CommandHandlerDelegate > CommandHandlers;
};

} // namespace Command
} // namespace IP