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

#include <IPShared/SlashCommands/SlashCommandInstance.h>

#include <IPShared/SlashCommands/SlashCommandDefinition.h>
#include <IPShared/SlashCommands/SlashCommandDataDefinition.h>
#include <IPCore/Utils/StringUtils.h>

namespace IP
{
namespace Command
{

CSlashCommandInstance::CSlashCommandInstance( void ) :
	Command( "" ),
	SubCommand( "" ),
	Params()
{
}


bool CSlashCommandInstance::Parse( const IP::String &command_line, const CSlashCommandDefinition *definition, IP::String &error_msg )
{
	const std::regex &param_matcher = definition->Get_Param_Match_Expression();

	std::cmatch param_match_results;
	std::regex_search( command_line.c_str(), param_match_results, param_matcher );

	const CSlashCommandDataDefinition *data_def = definition->Get_Data_Definition();
	uint32_t required_param_count = data_def->Get_Required_Param_Count();
	uint32_t total_param_count = data_def->Get_Param_Count();
	uint32_t param_start = data_def->Get_Match_Param_Start_Index();

	Command = param_match_results[ 1 ].str().c_str();
	if ( param_start > 2 )
	{
		SubCommand = param_match_results[ 2 ].str().c_str();
	}

	for ( uint32_t i = 0; i < total_param_count; ++i )
	{
		bool capture_remaining = data_def->Get_Param( i )->Should_Capture();

		IP::String param;
		uint32_t param_capture_index = 2 * i + param_start;
		if ( param_match_results.length( param_capture_index ) > 0 )
		{
			param = param_match_results.str( param_capture_index ).c_str();
		}
		else if ( !capture_remaining && param_match_results.length( param_capture_index + 1 ) > 0 )
		{
			param = param_match_results.str( param_capture_index + 1 ).c_str();
		}
		else if ( i >= required_param_count )
		{
			param = data_def->Get_Param( i )->Get_Default();
		}
		else
		{
			error_msg = "Insufficient number of parameters specified";
			return false;
		}

		if ( !data_def->Get_Param( i )->Is_Value_Valid( param ) )
		{
			error_msg = "Parameter value \"" + param + "\" is not valid.";
			return false;
		}

		Params.push_back( param );
	}

	return true;
}


void CSlashCommandInstance::Reset( void )
{
	Command = "";
	SubCommand = "";
	Params.clear();
}


bool CSlashCommandInstance::Get_Param( uint32_t index, int32_t &value ) const
{
	if ( index >= Params.size() )
	{
		return false;
	}

	return IP::StringUtils::Convert( Params[ index ], value );
}


bool CSlashCommandInstance::Get_Param( uint32_t index, uint32_t &value ) const
{
	if ( index >= Params.size() )
	{
		return false;
	}

	return IP::StringUtils::Convert( Params[ index ], value );
}


bool CSlashCommandInstance::Get_Param( uint32_t index, int64_t &value ) const
{
	if ( index >= Params.size() )
	{
		return false;
	}

	return IP::StringUtils::Convert( Params[ index ], value );
}


bool CSlashCommandInstance::Get_Param( uint32_t index, uint64_t &value ) const
{
	if ( index >= Params.size() )
	{
		return false;
	}

	return IP::StringUtils::Convert( Params[ index ], value );
}


bool CSlashCommandInstance::Get_Param( uint32_t index, IP::String &value ) const
{
	if ( index >= Params.size() )
	{
		return false;
	}

	return IP::StringUtils::Convert( Params[ index ], value );
}


bool CSlashCommandInstance::Get_Param( uint32_t index, float &value ) const
{
	if ( index >= Params.size() )
	{
		return false;
	}

	return IP::StringUtils::Convert( Params[ index ], value );
}


bool CSlashCommandInstance::Get_Param( uint32_t index, double &value ) const
{
	if ( index >= Params.size() )
	{
		return false;
	}

	return IP::StringUtils::Convert( Params[ index ], value );
}


bool CSlashCommandInstance::Get_Param( uint32_t index, bool &value ) const
{
	if ( index >= Params.size() )
	{
		return false;
	}

	return IP::StringUtils::Convert( Params[ index ], value );
}

} // namespace Command
} // namespace IP
