/**********************************************************************************************************************

	SlashCommandDefinition.cpp
		A component containing the definition of a slash command

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

/**********************************************************************************************************************
	CSlashCommandDefinition::CSlashCommandDefinition -- constructor

		data_definition -- the previously loaded static data for this command
	
**********************************************************************************************************************/
CSlashCommandDefinition::CSlashCommandDefinition( const CSlashCommandDataDefinition *data_definition ) :
	DataDefinition( data_definition ),
	ParamMatchExpression( data_definition != nullptr ? data_definition->Build_Command_Matcher() : L"" )
{
}