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

#include <IPShared/IPShared.h>

#include <IPCore/Memory/Stl/String.h>
#include <IPCore/Memory/Stl/Vector.h>

namespace IP
{
namespace Command
{

class CSlashCommandDefinition;

// A class representing an instance/invokation of a slash command.
IPSHARED_API class CSlashCommandInstance
{
	public:

		// Construction/destruction
		CSlashCommandInstance( void );
		~CSlashCommandInstance() {}

		// Operations
		bool Parse( const IP::String &command, const CSlashCommandDefinition *definition, IP::String &error_msg );

		void Reset( void );

		// Accessors
		const IP::String &Get_Command( void ) const { return Command; }
		const IP::String &Get_Sub_Command( void ) const { return SubCommand; }

		uint32_t Get_Param_Count( void ) { return static_cast< uint32_t >( Params.size() ); }

		bool Get_Param( uint32_t index, int32_t &value ) const;
		bool Get_Param( uint32_t index, uint32_t &value ) const;
		bool Get_Param( uint32_t index, int64_t &value ) const;
		bool Get_Param( uint32_t index, uint64_t &value ) const;
		bool Get_Param( uint32_t index, IP::String &value ) const;
		bool Get_Param( uint32_t index, float &value ) const;
		bool Get_Param( uint32_t index, double &value ) const;
		bool Get_Param( uint32_t index, bool &value ) const;

		// Todo: put an IsEnum compile-time restriction on this, compile-time assert on other path
		template< typename T >
		bool Get_Param( uint32_t index, T &value ) const
		{
			if ( index >= Params.size() )
			{
				return false;
			}

			return CEnumConverter::Convert( Params[ index ], value );
		}

	private:

		// Data
		IP::String Command;
		IP::String SubCommand;

		IP::Vector< IP::String > Params;

};

} // namespace Command
} // namespace IP