/**********************************************************************************************************************

	VirtualProcessProperties.cpp
		A component definining a struct describing the matchable properties of a virtual process

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

#include "VirtualProcessProperties.h"

/**********************************************************************************************************************
	SProcessProperties::Matches -- does the supplied property set match our implicit pattern?

		properties -- properties to check for a match against

		Returns: true if the properties matches our pattern, false otherwise
					
**********************************************************************************************************************/
bool SProcessProperties::Matches( const SProcessProperties &properties ) const
{
	// if the subjects don't match and our pattern does not include a subject wildcard then there is no match
	if ( properties.Value.Parts.Subject != Value.Parts.Subject && Value.Parts.Subject != 0 )
	{
		return false;
	}

	// if the major parts don't match and our pattern does not include a major part wildcard then there is no match
	if ( properties.Value.Parts.Major != Value.Parts.Major && Value.Parts.Major != 0 )
	{
		return false;
	}

	// if the minor parts don't match and our pattern does not include a minor part wildcard then there is no match
	if ( properties.Value.Parts.Minor != Value.Parts.Minor && Value.Parts.Minor != 0 )
	{
		return false;
	}

	// if the mode parts don't match and our pattern does not include a mode part wildcard then there is no match
	if ( properties.Value.Parts.Mode != Value.Parts.Mode && Value.Parts.Mode != 0 )
	{
		return false;
	}

	return true;
}
