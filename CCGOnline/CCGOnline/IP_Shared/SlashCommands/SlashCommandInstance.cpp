/**********************************************************************************************************************

	SlashCommandInstance.cpp
		A component containing the definition for an instance of a slash command

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
#include "StringUtils.h"

/**********************************************************************************************************************
	CSlashCommandInstance::CSlashCommandInstance -- default constructor
	
**********************************************************************************************************************/
CSlashCommandInstance::CSlashCommandInstance( void ) :
	Command( L"" ),
	SubCommand( L"" ),
	Params()
{
}

/**********************************************************************************************************************
	CSlashCommandInstance::Parse -- extracts all the parameters to a command

		command_line -- string to parse parameters from
		definition -- slash command this should be an instance of
		error_msg -- output parameter for what went wrong if an error occurred

	Returns: success/failure
	
**********************************************************************************************************************/
bool CSlashCommandInstance::Parse( const std::wstring &command_line, const CSlashCommandDefinition *definition, std::wstring &error_msg )
{
	const std::tr1::wregex &param_matcher = definition->Get_Param_Match_Expression();

	std::tr1::wcmatch param_match_results;
	std::tr1::regex_search( command_line.c_str(), param_match_results, param_matcher );

	const CSlashCommandDataDefinition *data_def = definition->Get_Data_Definition();
	uint32 required_param_count = data_def->Get_Required_Param_Count();
	uint32 total_param_count = data_def->Get_Param_Count();
	uint32 param_start = data_def->Get_Match_Param_Start_Index();

	Command = param_match_results[ 1 ];
	if ( param_start > 2 )
	{
		SubCommand = param_match_results[ 2 ];
	}

	for ( uint32 i = 0; i < total_param_count; ++i )
	{
		bool capture_remaining = data_def->Get_Param( i )->Should_Capture();

		std::wstring param;
		uint32 param_capture_index = 2 * i + param_start;
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

/**********************************************************************************************************************
	CSlashCommandInstance::Reset -- clears all command state from the instance
	
**********************************************************************************************************************/
void CSlashCommandInstance::Reset( void )
{
	Command = L"";
	SubCommand = L"";
	Params.clear();
}

/**********************************************************************************************************************
	CSlashCommandInstance::Get_Param -- queries a parameter value as an int32

		index -- which parameter to query
		value -- output variable for the parameter value

	Returns: success/failure
	
**********************************************************************************************************************/
bool CSlashCommandInstance::Get_Param( uint32 index, int32 &value ) const
{
	if ( index >= Params.size() )
	{
		return false;
	}

	return NStringUtils::Convert( Params[ index ], value );
}

/**********************************************************************************************************************
	CSlashCommandInstance::Get_Param -- queries a parameter value as an uint32

		index -- which parameter to query
		value -- output variable for the parameter value

	Returns: success/failure
	
**********************************************************************************************************************/
bool CSlashCommandInstance::Get_Param( uint32 index, uint32 &value ) const
{
	if ( index >= Params.size() )
	{
		return false;
	}

	return NStringUtils::Convert( Params[ index ], value );
}

/**********************************************************************************************************************
	CSlashCommandInstance::Get_Param -- queries a parameter value as an int64

		index -- which parameter to query
		value -- output variable for the parameter value

	Returns: success/failure
	
**********************************************************************************************************************/
bool CSlashCommandInstance::Get_Param( uint32 index, int64 &value ) const
{
	if ( index >= Params.size() )
	{
		return false;
	}

	return NStringUtils::Convert( Params[ index ], value );
}

/**********************************************************************************************************************
	CSlashCommandInstance::Get_Param -- queries a parameter value as an uint64

		index -- which parameter to query
		value -- output variable for the parameter value

	Returns: success/failure
	
**********************************************************************************************************************/
bool CSlashCommandInstance::Get_Param( uint32 index, uint64 &value ) const
{
	if ( index >= Params.size() )
	{
		return false;
	}

	return NStringUtils::Convert( Params[ index ], value );
}

/**********************************************************************************************************************
	CSlashCommandInstance::Get_Param -- queries a parameter value as a std::wstring

		index -- which parameter to query
		value -- output variable for the parameter value

	Returns: success/failure
	
**********************************************************************************************************************/
bool CSlashCommandInstance::Get_Param( uint32 index, std::wstring &value ) const
{
	if ( index >= Params.size() )
	{
		return false;
	}

	return NStringUtils::Convert( Params[ index ], value );
}

/**********************************************************************************************************************
	CSlashCommandInstance::Get_Param -- queries a parameter value as a std::string

		index -- which parameter to query
		value -- output variable for the parameter value

	Returns: success/failure
	
**********************************************************************************************************************/
bool CSlashCommandInstance::Get_Param( uint32 index, std::string &value ) const
{
	if ( index >= Params.size() )
	{
		return false;
	}

	return NStringUtils::Convert( Params[ index ], value );
}

/**********************************************************************************************************************
	CSlashCommandInstance::Get_Param -- queries a parameter value as a float

		index -- which parameter to query
		value -- output variable for the parameter value

	Returns: success/failure
	
**********************************************************************************************************************/
bool CSlashCommandInstance::Get_Param( uint32 index, float &value ) const
{
	if ( index >= Params.size() )
	{
		return false;
	}

	return NStringUtils::Convert( Params[ index ], value );
}

/**********************************************************************************************************************
	CSlashCommandInstance::Get_Param -- queries a parameter value as a double

		index -- which parameter to query
		value -- output variable for the parameter value

	Returns: success/failure
	
**********************************************************************************************************************/
bool CSlashCommandInstance::Get_Param( uint32 index, double &value ) const
{
	if ( index >= Params.size() )
	{
		return false;
	}

	return NStringUtils::Convert( Params[ index ], value );
}

/**********************************************************************************************************************
	CSlashCommandInstance::Get_Param -- queries a parameter value as a bool

		index -- which parameter to query
		value -- output variable for the parameter value

	Returns: success/failure
	
**********************************************************************************************************************/
bool CSlashCommandInstance::Get_Param( uint32 index, bool &value ) const
{
	if ( index >= Params.size() )
	{
		return false;
	}

	return NStringUtils::Convert( Params[ index ], value );
}

