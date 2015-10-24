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

#include "SlashCommandDefinition.h"
#include "SlashCommandDataDefinition.h"
#include "SlashCommandInstance.h"
#include "IPPlatform/StringUtils.h"

namespace IP
{
namespace Command
{

CSlashCommandInstance::CSlashCommandInstance( void ) :
	Command( L"" ),
	SubCommand( L"" ),
	Params()
{
}


bool CSlashCommandInstance::Parse( const std::wstring &command_line, const CSlashCommandDefinition *definition, std::wstring &error_msg )
{
	const std::tr1::wregex &param_matcher = definition->Get_Param_Match_Expression();

	std::tr1::wcmatch param_match_results;
	std::tr1::regex_search( command_line.c_str(), param_match_results, param_matcher );

	const CSlashCommandDataDefinition *data_def = definition->Get_Data_Definition();
	uint32_t required_param_count = data_def->Get_Required_Param_Count();
	uint32_t total_param_count = data_def->Get_Param_Count();
	uint32_t param_start = data_def->Get_Match_Param_Start_Index();

	Command = param_match_results[ 1 ];
	if ( param_start > 2 )
	{
		SubCommand = param_match_results[ 2 ];
	}

	for ( uint32_t i = 0; i < total_param_count; ++i )
	{
		bool capture_remaining = data_def->Get_Param( i )->Should_Capture();

		std::wstring param;
		uint32_t param_capture_index = 2 * i + param_start;
		if ( param_match_results.length( param_capture_index ) > 0 )
		{
			param = param_match_results.str( param_capture_index );
		}
		else if ( !capture_remaining && param_match_results.length( param_capture_index + 1 ) > 0 )
		{
			param = param_match_results.str( param_capture_index + 1 );
		}
		else if ( i >= required_param_count )
		{
			param = data_def->Get_Param( i )->Get_Default();
		}
		else
		{
			error_msg = L"Insufficient number of parameters specified";
			return false;
		}

		if ( !data_def->Get_Param( i )->Is_Value_Valid( param ) )
		{
			error_msg = L"Parameter value \"" + param + L"\" is not valid.";
			return false;
		}

		Params.push_back( param );
	}

	return true;
}


void CSlashCommandInstance::Reset( void )
{
	Command = L"";
	SubCommand = L"";
	Params.clear();
}


bool CSlashCommandInstance::Get_Param( uint32_t index, int32_t &value ) const
{
	if ( index >= Params.size() )
	{
		return false;
	}

	return IP::String::Convert( Params[ index ], value );
}


bool CSlashCommandInstance::Get_Param( uint32_t index, uint32_t &value ) const
{
	if ( index >= Params.size() )
	{
		return false;
	}

	return IP::String::Convert( Params[ index ], value );
}


bool CSlashCommandInstance::Get_Param( uint32_t index, int64_t &value ) const
{
	if ( index >= Params.size() )
	{
		return false;
	}

	return IP::String::Convert( Params[ index ], value );
}


bool CSlashCommandInstance::Get_Param( uint32_t index, uint64_t &value ) const
{
	if ( index >= Params.size() )
	{
		return false;
	}

	return IP::String::Convert( Params[ index ], value );
}


bool CSlashCommandInstance::Get_Param( uint32_t index, std::wstring &value ) const
{
	if ( index >= Params.size() )
	{
		return false;
	}

	return IP::String::Convert( Params[ index ], value );
}


bool CSlashCommandInstance::Get_Param( uint32_t index, std::string &value ) const
{
	if ( index >= Params.size() )
	{
		return false;
	}

	return IP::String::Convert( Params[ index ], value );
}


bool CSlashCommandInstance::Get_Param( uint32_t index, float &value ) const
{
	if ( index >= Params.size() )
	{
		return false;
	}

	return IP::String::Convert( Params[ index ], value );
}


bool CSlashCommandInstance::Get_Param( uint32_t index, double &value ) const
{
	if ( index >= Params.size() )
	{
		return false;
	}

	return IP::String::Convert( Params[ index ], value );
}


bool CSlashCommandInstance::Get_Param( uint32_t index, bool &value ) const
{
	if ( index >= Params.size() )
	{
		return false;
	}

	return IP::String::Convert( Params[ index ], value );
}

} // namespace Command
} // namespace IP
