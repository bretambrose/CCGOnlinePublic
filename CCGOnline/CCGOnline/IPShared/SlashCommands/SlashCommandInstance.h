/**********************************************************************************************************************

	SlashCommandInstance.h
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

#ifndef SLASH_COMMAND_INSTANCE_H
#define SLASH_COMMAND_INSTANCE_H

class CSlashCommandDefinition;

// A class representing an instance/invokation of a slash command.
class CSlashCommandInstance
{
	public:

		// Construction/destruction
		CSlashCommandInstance( void );
		~CSlashCommandInstance() {}

		// Operations
		bool Parse( const std::wstring &command, const CSlashCommandDefinition *definition, std::wstring &error_msg );

		void Reset( void );

		// Accessors
		const std::wstring &Get_Command( void ) const { return Command; }
		const std::wstring &Get_Sub_Command( void ) const { return SubCommand; }

		uint32 Get_Param_Count( void ) { return static_cast< uint32 >( Params.size() ); }

		bool Get_Param( uint32 index, int32 &value ) const;
		bool Get_Param( uint32 index, uint32 &value ) const;
		bool Get_Param( uint32 index, int64 &value ) const;
		bool Get_Param( uint32 index, uint64 &value ) const;
		bool Get_Param( uint32 index, std::wstring &value ) const;
		bool Get_Param( uint32 index, std::string &value ) const;
		bool Get_Param( uint32 index, float &value ) const;
		bool Get_Param( uint32 index, double &value ) const;
		bool Get_Param( uint32 index, bool &value ) const;

		// Todo: put an IsEnum compile-time restriction on this, compile-time assert on other path
		template< typename T >
		bool Get_Param( uint32 index, T &value ) const
		{
			if ( index >= Params.size() )
			{
				return false;
			}

			return CEnumConverter::Convert( Params[ index ], value );
		}

	private:

		// Data
		std::wstring Command;
		std::wstring SubCommand;

		std::vector< std::wstring > Params;

};

#endif // SLASH_COMMAND_INSTANCE_H
