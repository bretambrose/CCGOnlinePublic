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

	SCPT_INT32,				//:EnumEntry( "int32" )
	SCPT_UINT32,			//:EnumEntry( "uint32" )
	SCPT_INT64,				//:EnumEntry( "int64" )
	SCPT_UINT64,			//:EnumEntry( "uint64" )
	SCPT_STRING,			//:EnumEntry( "string" )
	SCPT_WIDE_STRING,		//:EnumEntry( "wstring" )
	SCPT_FLOAT,				//:EnumEntry( "float" )
	SCPT_DOUBLE,			//:EnumEntry( "double" )
	SCPT_BOOLEAN,			//:EnumEntry( "bool" )
	SCPT_ENUM				//:EnumEntry( "enum" )

};
//:EnumEnd

class CSlashCommandParam
{
	public:

		CSlashCommandParam( void );
		~CSlashCommandParam() {}

		static IXMLSerializer *Create_Serializer( void );

		ESlashCommandParamType Get_Type( void ) const { return Type; }
		const std::string &Get_Sub_Type( void ) const { return SubType; }
		const std::wstring &Get_Default( void ) const { return Default; }

		bool Is_Optional( void ) const { return Optional; }
		bool Should_Capture( void ) const { return Capture; }

		bool Is_Value_Valid( const std::wstring &value ) const;

		void Initialize_Default( void );
		bool Is_Default_Valid( void ) const { return Is_Value_Valid( Default ); }

	private:

		ESlashCommandParamType Type;
		std::string SubType;
		std::wstring Default;

		bool Optional;
		bool Capture;
};

class CSlashCommandDataDefinition
{
	public:

		CSlashCommandDataDefinition( void );
		~CSlashCommandDataDefinition() {}

		void Post_Load_XML( void );

		static IXMLSerializer *Create_Serializer( void );

		const std::wstring &Get_Command( void ) const { return Command; }
		const std::wstring &Get_Sub_Command( void ) const { return SubCommand; }
		const std::wstring &Get_Shortcut( void ) const { return Shortcut; }
		const std::wstring &Get_Help( void ) const { return Help; }

		uint32 Get_Match_Param_Start_Index( void ) const { return SubCommand.size() == 0 ? 2 : 3; }
		uint32 Get_Required_Param_Count( void ) const { return RequiredParamCount; }
		uint32 Get_Total_Capture_Group_Count( void ) const { return TotalCaptureGroupCount; }

		uint32 Get_Param_Count( void ) const { return static_cast< uint32 >( Params.size() ); }
		const CSlashCommandParam *Get_Param( uint32 index ) const;

		const std::wstring &Get_Key( void ) const { return Key; }

		std::wstring Build_Command_Matcher( void ) const;

	private:

		std::wstring Command;
		std::wstring SubCommand;
		std::wstring Shortcut;
		std::wstring Help;

		// unserialized
		std::wstring Key;

		std::vector< CSlashCommandParam > Params;

		uint32 RequiredParamCount;
		uint32 TotalCaptureGroupCount;
};

#endif // SLASH_COMMAND_DEFINITION_H