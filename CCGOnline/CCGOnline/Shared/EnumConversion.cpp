/**********************************************************************************************************************

	EnumConversion.cpp
		A static class that all convertible enums register with, used to convert back and forth between
		string and numeric representations.		

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

#include "EnumConversion.h"
#include "StringUtils.h"

class CConvertibleEnum 
{
	public:

		CConvertibleEnum( const std::string &name, EConvertibleEnumProperties properties );

		const std::string &Get_Name( void ) const { return Name; }

		EConvertibleEnumProperties Get_Properties( void ) const { return Properties; }

		bool Convert( const std::string &entry_name, uint64 &output_value ) const;
		bool Convert( uint64 output_value, std::string &entry_name ) const;

		void Register_Entry( const std::string &entry_name, uint64 value );

	private:

		bool Convert_Internal( const std::string &entry_name, uint64 &output_value ) const;
		bool Convert_Internal( uint64 value, std::string &entry_name ) const;

		bool Convert_Bitfield_Internal( const std::string &mask_name, uint64 &output_value ) const;
		bool Convert_Bitfield_Internal( uint64 value, std::string &mask_name ) const;

		std::string Name;

		EConvertibleEnumProperties Properties;

		stdext::hash_map< std::string, uint64 > NameToValueTable;
		stdext::hash_map< uint64, std::string > ValueToNameTable;

};

CConvertibleEnum::CConvertibleEnum( const std::string &name, EConvertibleEnumProperties properties ) :
	Name( name ),
	Properties( properties ),
	NameToValueTable(),
	ValueToNameTable()
{
}

bool CConvertibleEnum::Convert( const std::string &entry_name, uint64 &output_value ) const
{
	if ( ( Properties & CEP_BITFIELD ) != 0 )
	{
		return Convert_Bitfield_Internal( entry_name, output_value );
	}
	else
	{
		return Convert_Internal( entry_name, output_value );
	}
}

bool CConvertibleEnum::Convert( uint64 output_value, std::string &entry_name ) const
{
	if ( ( Properties & CEP_BITFIELD ) != 0 )
	{
		return Convert_Bitfield_Internal( output_value, entry_name );
	}
	else
	{
		return Convert_Internal( output_value, entry_name );
	}
}

void CConvertibleEnum::Register_Entry( const std::string &entry_name, uint64 value )
{
	std::string upper_entry_name;
	NStringUtils::To_Upper_Case( entry_name, upper_entry_name );

	auto name_iter = NameToValueTable.find( upper_entry_name );
	FATAL_ASSERT( name_iter == NameToValueTable.end() );

	auto value_iter = ValueToNameTable.find( value );
	FATAL_ASSERT( value_iter == ValueToNameTable.end() );

	NameToValueTable[ upper_entry_name ] = value;
	ValueToNameTable[ value ] = upper_entry_name;
}

bool CConvertibleEnum::Convert_Internal( const std::string &entry_name, uint64 &output_value ) const
{
	std::string upper_entry_name;
	NStringUtils::To_Upper_Case( entry_name, upper_entry_name );

	auto iter = NameToValueTable.find( upper_entry_name );
	if ( iter == NameToValueTable.end() )
	{
		return false;
	}

	output_value = iter->second;
	return true;
}

bool CConvertibleEnum::Convert_Internal( uint64 value, std::string &entry_name ) const
{
	auto iter = ValueToNameTable.find( value );
	if ( iter == ValueToNameTable.end() )
	{
		return false;
	}

	entry_name = iter->second;
	return true;
}

static uint32 Skip_Separators( const char *string_buffer, uint32 index )
{
	char current_char = *( string_buffer + index );
	while ( current_char != 0 && ( current_char == ' ' || current_char == '\t' || current_char == '|' ) )
	{
		current_char = *( string_buffer + ++index );
	}

	return index;
}

static uint32 Skip_Non_Separators( const char *string_buffer, uint32 index )
{
	char current_char = *( string_buffer + index );
	while ( current_char != 0 && ( current_char != ' ' && current_char != '\t' && current_char != '|' ) )
	{
		current_char = *( string_buffer + ++index );
	}

	return index;
}

bool CConvertibleEnum::Convert_Bitfield_Internal( const std::string &mask_name, uint64 &output_value ) const
{
	const char *raw_characters = mask_name.c_str();
	output_value = 0;

	uint32 entry_start = Skip_Separators( raw_characters, 0 );
	while ( entry_start < mask_name.size() )
	{
		uint32 entry_end = Skip_Non_Separators( raw_characters, entry_start );

		std::string entry( raw_characters + entry_start, raw_characters + entry_end );
		uint64 entry_value = 0;

		if ( !Convert_Internal( entry, entry_value ) )
		{
			return false;
		}

		output_value |= entry_value;

		entry_start = Skip_Separators( raw_characters, entry_end );
	}

	return true;
}

bool CConvertibleEnum::Convert_Bitfield_Internal( uint64 value, std::string &mask_name ) const
{
	if ( value == 0 )
	{
		return Convert_Internal( value, mask_name );
	}

	mask_name = "";
	uint64 value_iterator = value;
	bool first_entry = true;

	for ( uint32 bit_index = 0; value_iterator > 0; bit_index++, value_iterator >>= 1 )
	{
		if ( ( value_iterator & 0x01 ) != 0 )
		{
			uint64 bit_value = 1UL << bit_index;

			std::string entry_name;
			if ( !Convert_Internal( bit_value, entry_name ) )
			{
				return false;
			}

			if ( first_entry )
			{
				mask_name = mask_name + entry_name;
			}
			else
			{
				mask_name = mask_name + " | " + entry_name;
			}

			first_entry = false;
		}
	}

	return true;
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

stdext::hash_map< std::string, CConvertibleEnum * > CEnumConverter::Enums;

void CEnumConverter::Cleanup( void )
{
	for ( auto iter = Enums.begin(); iter != Enums.end(); ++iter )
	{
		delete iter->second;
	}

	Enums.clear();
}

void CEnumConverter::Register_Enum( const std::string &enum_name, EConvertibleEnumProperties properties )
{
	std::string upper_enum_name;
	NStringUtils::To_Upper_Case( enum_name, upper_enum_name );

	FATAL_ASSERT( Enums.find( upper_enum_name ) == Enums.end() );

	CConvertibleEnum *enum_object = new CConvertibleEnum( enum_name, properties );
	Enums[ upper_enum_name ] = enum_object;
}
		
void CEnumConverter::Register_Enum_Entry( const std::string &enum_name, const std::string &entry_name, uint64 entry_value )
{
	CConvertibleEnum *enum_object = Find_Enum( enum_name );
	FATAL_ASSERT( enum_object != nullptr );

	enum_object->Register_Entry( entry_name, entry_value );
}

CConvertibleEnum *CEnumConverter::Find_Enum( const std::string &enum_name )
{
	std::string upper_enum_name;
	NStringUtils::To_Upper_Case( enum_name, upper_enum_name );

	auto iter = Enums.find( upper_enum_name );
	if ( iter == Enums.end() )
	{
		return nullptr;
	}

	return iter->second;
}

bool CEnumConverter::Convert_Internal( const std::string &enum_name, const std::string &entry_name, uint64 &output_value )
{
	CConvertibleEnum *enum_object = Find_Enum( enum_name );
	if ( enum_object == nullptr )
	{
		return false;
	}

	return enum_object->Convert( entry_name, output_value );
}

bool CEnumConverter::Convert_Internal( const std::string &enum_name, uint64 output_value, std::string &entry_name )
{
	CConvertibleEnum *enum_object = Find_Enum( enum_name );
	if ( enum_object == nullptr )
	{
		return false;
	}

	return enum_object->Convert( output_value, entry_name );
}

