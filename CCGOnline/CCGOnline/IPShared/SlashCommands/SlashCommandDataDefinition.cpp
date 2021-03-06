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

#include "stdafx.h"

#include "SlashCommandDataDefinition.h"
#include "IPShared/Serialization/XML/PrimitiveXMLSerializers.h"
#include "SlashCommandManager.h"
#include "IPPlatform/StringUtils.h"
#include "IPShared/Serialization/SerializationRegistrar.h"

namespace IP
{
namespace Command
{

CSlashCommandParam::CSlashCommandParam( void ) :
	Type( SCPT_INVALID ),
	SubType( "" ),
	Default( L"" ),
	Optional( false ),
	Capture( false )
{
}


void CSlashCommandParam::Register_Type_Definition( void )
{
	BEGIN_ROOT_TYPE_DEFINITION( CSlashCommandParam );

	REGISTER_MEMBER_BINDING( L"Type", &CSlashCommandParam::Type );
	REGISTER_MEMBER_BINDING( L"SubType", &CSlashCommandParam::SubType );
	REGISTER_MEMBER_BINDING( L"Default", &CSlashCommandParam::Default );
	REGISTER_MEMBER_BINDING( L"Optional", &CSlashCommandParam::Optional );
	REGISTER_MEMBER_BINDING( L"Capture", &CSlashCommandParam::Capture );

	END_TYPE_DEFINITION( CSlashCommandParam );
}


bool CSlashCommandParam::Is_Value_Valid( const std::wstring &value ) const
{
	switch ( Type )
	{
		case SCPT_INT32:
		{
			int32_t converted_value = 0;
			return IP::String::Convert( value, converted_value );
		}

		case SCPT_UINT32:
		{
			uint32_t converted_value = 0;
			return IP::String::Convert( value, converted_value );
		}

		case SCPT_INT64:
		{
			int64_t converted_value = 0;
			return IP::String::Convert( value, converted_value );
		}

		case SCPT_UINT64:
		{
			uint64_t converted_value = 0;
			return IP::String::Convert( value, converted_value );
		}

		case SCPT_STRING:
		{
			std::string converted_value;
			return IP::String::Convert( value, converted_value );
		}

		case SCPT_WIDE_STRING:
		{
			std::wstring converted_value;
			return IP::String::Convert( value, converted_value );
		}

		case SCPT_FLOAT:
		{
			float converted_value = 0.0f;
			return IP::String::Convert( value, converted_value );
		}

		case SCPT_DOUBLE:
		{
			double converted_value = 0.0;
			return IP::String::Convert( value, converted_value );
		}

		case SCPT_BOOLEAN:
		{
			bool converted_value = false;
			return IP::String::Convert( value, converted_value );
		}

		case SCPT_ENUM:
		{
			uint64_t converted_value = 0;
			return IP::Enum::CEnumConverter::Convert( SubType, value, converted_value );
		}

		default:
			return false;	
	}
}


void CSlashCommandParam::Initialize_Default( void )
{
	if ( Default.size() > 0 )
	{
		return;
	}

	switch ( Type )
	{
		case SCPT_INT32:
		case SCPT_UINT32:
		case SCPT_INT64:
		case SCPT_UINT64:
			Default = L"0";
			break;

		case SCPT_STRING:
		case SCPT_WIDE_STRING:
			break;

		case SCPT_FLOAT:
		case SCPT_DOUBLE:
			Default = L"0.0";
			break;

		case SCPT_BOOLEAN:
			Default = L"false";
			break;

		case SCPT_ENUM:
		default:
			FATAL_ASSERT( false );
			break;
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


CSlashCommandDataDefinition::CSlashCommandDataDefinition( void ) :
	Command( L"" ),
	SubCommand( L"" ),
	Shortcut( L"" ),
	Help( L"" ),
	Key( L"" ),
	Params(),
	RequiredParamCount( 0 ),
	TotalCaptureGroupCount( 0 )
{
}


void CSlashCommandDataDefinition::Register_Type_Definition( void )
{
	BEGIN_ROOT_TYPE_DEFINITION( CSlashCommandDataDefinition );

	REGISTER_MEMBER_BINDING( L"Command", &CSlashCommandDataDefinition::Command );
	REGISTER_MEMBER_BINDING( L"SubCommand", &CSlashCommandDataDefinition::SubCommand );
	REGISTER_MEMBER_BINDING( L"Shortcut", &CSlashCommandDataDefinition::Shortcut );
	REGISTER_MEMBER_BINDING( L"Help", &CSlashCommandDataDefinition::Help );
	REGISTER_MEMBER_BINDING( L"Params", &CSlashCommandDataDefinition::Params );

	END_TYPE_DEFINITION( CSlashCommandDataDefinition );
}


const CSlashCommandParam *CSlashCommandDataDefinition::Get_Param( uint32_t index ) const
{
	if ( index < Params.size() )
	{
		return &Params[ index ];
	}

	return nullptr;
}


void CSlashCommandDataDefinition::Post_Load_XML( void )
{
	std::wstring key = CSlashCommandManager::Concat_Command( Command, SubCommand );
	IP::String::To_Upper_Case( key, Key );

	TotalCaptureGroupCount = Get_Match_Param_Start_Index();

	// Validate default value, optional, and capture remaining
	bool optional_encountered = false;
	for ( uint32_t i = 0; i < Params.size(); ++i )
	{
		CSlashCommandParam &param = Params[ i ];
		bool is_optional = param.Is_Optional();
		if ( is_optional )
		{
			optional_encountered = true;
		}

		bool capture_remaining = param.Should_Capture();

		FATAL_ASSERT( !capture_remaining || i + 1 == Params.size() );	//	A capture remaining parameter may only be the last parameter in the list
		FATAL_ASSERT( !( optional_encountered && !is_optional ) );		// Once an optional parameter is encountered, all succeeding parameters must be optional

		param.Initialize_Default();
		FATAL_ASSERT( param.Is_Default_Valid() );								// Make sure the default makes sense

		if ( !is_optional )
		{
			RequiredParamCount++;
		}

		// The regular expression per parameter normally has two capture groups, but the capture remaining expression only has 1
		TotalCaptureGroupCount += capture_remaining ? 1 : 2;
	}
}


std::wstring CSlashCommandDataDefinition::Build_Command_Matcher( void ) const
{
	std::wstring match_string;

	if ( SubCommand.size() == 0 )
	{
		match_string = L"^/(\\w+)";
	}
	else
	{
		match_string = L"^/(\\w+)\\s+(\\w+)";
	}

	for ( uint32_t i = 0; i < Params.size(); ++i )
	{
		const CSlashCommandParam &param = Params[ i ];
		bool is_required = !param.Is_Optional();
		bool consume_remaining = param.Should_Capture();

		if ( is_required )
		{
			if ( consume_remaining )
			{
				match_string.append( L"\\s+([^\\r\\n]*)" );
			}
			else
			{
				match_string.append( L"(?:\\s+(?:(?:\"(.*?)\")|(\\S+)))" );
			}
		}
		else
		{
			match_string.append( L"(?:\\s+(?:(?:\"(.*?)\")|(\\S+)))?" );
		}
	}

	return match_string;
}

} // namespace Command
} // namespace IP
