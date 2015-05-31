/**********************************************************************************************************************

	SlashCommandDataDefinition.h
		A component containing the static data of a slash command

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

#ifndef SLASH_COMMAND_DATA_DEFINITION_H
#define SLASH_COMMAND_DATA_DEFINITION_H

class IXMLSerializer;

//:EnumBegin()
enum ESlashCommandParamType
{
	SCPT_INVALID,

	SCPT_INT32,				//:EnumEntry( "int32_t" )
	SCPT_UINT32,			//:EnumEntry( "uint32_t" )
	SCPT_INT64,				//:EnumEntry( "int64_t" )
	SCPT_UINT64,			//:EnumEntry( "uint64_t" )
	SCPT_STRING,			//:EnumEntry( "string" )
	SCPT_WIDE_STRING,		//:EnumEntry( "wstring" )
	SCPT_FLOAT,				//:EnumEntry( "float" )
	SCPT_DOUBLE,			//:EnumEntry( "double" )
	SCPT_BOOLEAN,			//:EnumEntry( "bool" )
	SCPT_ENUM				//:EnumEntry( "enum" )

};
//:EnumEnd

// A class containing the static data of a single parameter to a slash command
class CSlashCommandParam
{
	public:

		// Construction/Destruction
		CSlashCommandParam( void );
		~CSlashCommandParam() {}

		static void Register_Type_Definition( void );

		// Accessors
		ESlashCommandParamType Get_Type( void ) const { return Type; }
		const std::string &Get_Sub_Type( void ) const { return SubType; }
		const std::wstring &Get_Default( void ) const { return Default; }

		bool Is_Optional( void ) const { return Optional; }
		bool Should_Capture( void ) const { return Capture; }

		// Validation and Defaults
		bool Is_Value_Valid( const std::wstring &value ) const;

		void Initialize_Default( void );
		bool Is_Default_Valid( void ) const { return Is_Value_Valid( Default ); }

	private:

		// Data
		ESlashCommandParamType Type;
		std::string SubType;		// If Type is SCPT_ENUM, this contains the name of the enum
		std::wstring Default;	// Default value override for this parameter

		bool Optional;				// Is this parameter optional?
		bool Capture;				// Should this parameter capture all remaining input?  As an example, think of the chat part of a chat command.
};

// A class containing the static data defining a slash command
class CSlashCommandDataDefinition
{
	public:

		// Construction/Destruction
		CSlashCommandDataDefinition( void );
		~CSlashCommandDataDefinition() {}

		// Serialization
		void Post_Load_XML( void );

		static void Register_Type_Definition( void );

		// Accessors
		const std::wstring &Get_Command( void ) const { return Command; }
		const std::wstring &Get_Sub_Command( void ) const { return SubCommand; }
		const std::wstring &Get_Shortcut( void ) const { return Shortcut; }
		const std::wstring &Get_Help( void ) const { return Help; }

		uint32_t Get_Match_Param_Start_Index( void ) const { return SubCommand.size() == 0 ? 2 : 3; }
		uint32_t Get_Required_Param_Count( void ) const { return RequiredParamCount; }
		uint32_t Get_Total_Capture_Group_Count( void ) const { return TotalCaptureGroupCount; }

		uint32_t Get_Param_Count( void ) const { return static_cast< uint32_t >( Params.size() ); }
		const CSlashCommandParam *Get_Param( uint32_t index ) const;

		const std::wstring &Get_Key( void ) const { return Key; }

		std::wstring Build_Command_Matcher( void ) const;

	private:

		std::wstring Command;
		std::wstring SubCommand;
		std::wstring Shortcut;
		std::wstring Help;

		std::wstring Key;		// unserialized, derived from Command and SubCommand

		std::vector< CSlashCommandParam > Params;

		uint32_t RequiredParamCount;				// unserialized, derived from Params properties
		uint32_t TotalCaptureGroupCount;		// unserialized, derived from various properties
};

#endif // SLASH_COMMAND_DEFINITION_H
