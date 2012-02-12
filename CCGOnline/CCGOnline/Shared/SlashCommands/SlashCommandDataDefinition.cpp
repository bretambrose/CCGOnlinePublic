/**********************************************************************************************************************

	SlashCommandDataDefinition.cpp
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

#include "stdafx.h"

#include "SlashCommandDataDefinition.h"
#include "XML/PrimitiveXMLSerializers.h"
#include "SlashCommandManager.h"
#include "StringUtils.h"

CSlashCommandParam::CSlashCommandParam( void ) :
	Type( SCPT_INVALID ),
	SubType( "" ),
	Default( L"" ),
	Optional( false ),
	Capture( false )
{
}

IXMLSerializer *CSlashCommandParam::Create_Serializer( void )
{
	CCompositeXMLSerializer *serializer = new CCompositeXMLSerializer;

	serializer->Add( L"Type", &CSlashCommandParam::Type );
	serializer->Add( L"SubType", &CSlashCommandParam::SubType );
	serializer->Add( L"Default", &CSlashCommandParam::Default );
	serializer->Add( L"Optional", &CSlashCommandParam::Optional );
	serializer->Add( L"Capture", &CSlashCommandParam::Capture );

	return serializer;
}

bool CSlashCommandParam::Is_Value_Valid( const std::wstring &value ) const
{
	switch ( Type )
	{
		case SCPT_INT32:
		{
			int32 converted_value = 0;
			return NStringUtils::Convert( value, converted_value );
		}

		case SCPT_UINT32:
		{
			uint32 converted_value = 0;
			return NStringUtils::Convert( value, converted_value );
		}

		case SCPT_INT64:
		{
			int64 converted_value = 0;
			return NStringUtils::Convert( value, converted_value );
		}

		case SCPT_UINT64:
		{
			uint64 converted_value = 0;
			return NStringUtils::Convert( value, converted_value );
		}

		case SCPT_STRING:
		{
			std::string converted_value;
			return NStringUtils::Convert( value, converted_value );
		}

		case SCPT_WIDE_STRING:
		{
			std::wstring converted_value;
			return NStringUtils::Convert( value, converted_value );
		}

		case SCPT_FLOAT:
		{
			float converted_value = 0.0f;
			return NStringUtils::Convert( value, converted_value );
		}

		case SCPT_DOUBLE:
		{
			double converted_value = 0.0;
			return NStringUtils::Convert( value, converted_value );
		}

		case SCPT_BOOLEAN:
		{
			bool converted_value = false;
			return NStringUtils::Convert( value, converted_value );
		}

		case SCPT_ENUM:
		{
			uint64 converted_value = 0;
			return CEnumConverter::Convert( SubType, value, converted_value );
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


IXMLSerializer *CSlashCommandDataDefinition::Create_Serializer( void )
{
	CCompositeXMLSerializer *serializer = new CCompositeXMLSerializer;

	serializer->Add( L"Command", &CSlashCommandDataDefinition::Command );
	serializer->Add( L"SubCommand", &CSlashCommandDataDefinition::SubCommand );
	serializer->Add( L"Shortcut", &CSlashCommandDataDefinition::Shortcut );
	serializer->Add( L"Help", &CSlashCommandDataDefinition::Help );

	serializer->Add( L"Params", &CSlashCommandDataDefinition::Params, new CVectorXMLSerializer< CSlashCommandParam > );

	return serializer;
}

const CSlashCommandParam *CSlashCommandDataDefinition::Get_Param( uint32 index ) const
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
	NStringUtils::To_Upper_Case( key, Key );

	TotalCaptureGroupCount = Get_Match_Param_Start_Index();

	// Validate optional and capture remaining
	bool optional_encountered = false;
	for ( uint32 i = 0; i < Params.size(); ++i )
	{
		CSlashCommandParam &param = Params[ i ];
		bool is_optional = param.Is_Optional();
		if ( is_optional )
		{
			optional_encountered = true;
		}

		bool capture_remaining = param.Should_Capture();

		FATAL_ASSERT( !capture_remaining || i + 1 == Params.size() );
		FATAL_ASSERT( !( optional_encountered && !is_optional ) );

		param.Initialize_Default();
		FATAL_ASSERT( param.Is_Default_Valid() );

		if ( !is_optional )
		{
			RequiredParamCount++;
		}

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

	for ( uint32 i = 0; i < Params.size(); ++i )
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


