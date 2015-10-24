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

#include <regex>

namespace IP
{
namespace Command
{

class CSlashCommandDataDefinition;

// A wrapper class for all static data; we split the pattern matching regex away from the rest of the derived data
// so that a definition with a null data definition can represent a command family (clumsy)
class CSlashCommandDefinition
{
	public:

		// Construction/destruction
		CSlashCommandDefinition( const CSlashCommandDataDefinition *data_definition );
		~CSlashCommandDefinition() {}

		// Accessors
		const CSlashCommandDataDefinition *Get_Data_Definition( void ) const { return DataDefinition; }

		bool Is_Family( void ) const { return DataDefinition == nullptr; }

		const std::tr1::wregex &Get_Param_Match_Expression( void ) const { return ParamMatchExpression; }

	private:

		// Data
		const CSlashCommandDataDefinition *DataDefinition;

		std::tr1::wregex ParamMatchExpression;
};

} // namespace Command
} // namespace IP
