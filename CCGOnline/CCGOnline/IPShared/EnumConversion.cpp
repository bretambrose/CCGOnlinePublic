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
#include "IPPlatform/StringUtils.h"

// A simple, hidden class that contains all the conversion information for a single enum
class CConvertibleEnum 
{
	public:

		// Construction
		CConvertibleEnum( const std::string &name, EConvertibleEnumProperties properties );

		// Accessors
		const std::string &Get_Name( void ) const { return Name; }
		EConvertibleEnumProperties Get_Properties( void ) const { return Properties; }

		// Conversion registration
		void Register_Entry( const std::string &entry_name, uint64_t value );

		// Conversion
		bool Convert( const std::string &entry_name, uint64_t &output_value ) const;
		bool Convert( uint64_t output_value, std::string &entry_name ) const;

	private:

		bool Convert_Internal( const std::string &entry_name, uint64_t &output_value ) const;
		bool Convert_Internal( uint64_t value, std::string &entry_name ) const;

		bool Convert_Bitfield_Internal( const std::string &mask_name, uint64_t &output_value ) const;
		bool Convert_Bitfield_Internal( uint64_t value, std::string &mask_name ) const;

		// Data
		std::string Name;

		EConvertibleEnumProperties Properties;

		std::unordered_map< std::string, uint64_t > NameToValueTable;
		std::unordered_map< uint64_t, std::string > ValueToNameTable;
};

/**********************************************************************************************************************
	CConvertibleEnum::CConvertibleEnum -- constructor

		name -- name of the enum
		properties -- any special properties the enum has (bitfield)

**********************************************************************************************************************/
CConvertibleEnum::CConvertibleEnum( const std::string &name, EConvertibleEnumProperties properties ) :
	Name( name ),
	Properties( properties ),
	NameToValueTable(),
	ValueToNameTable()
{
}

/**********************************************************************************************************************
	CConvertibleEnum::Convert -- top level function to convert between a string and an integer value

		entry_name -- string to convert from
		output_value -- output parameter for the integer value

		Returns: success/failure

**********************************************************************************************************************/
bool CConvertibleEnum::Convert( const std::string &entry_name, uint64_t &output_value ) const
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

/**********************************************************************************************************************
	CConvertibleEnum::Convert -- top level function to convert between an integer value and a string

		value -- integer to convert from
		entry_name -- output parameter for the corresponding string value

		Returns: success/failure

**********************************************************************************************************************/
bool CConvertibleEnum::Convert( uint64_t value, std::string &entry_name ) const
{
	if ( ( Properties & CEP_BITFIELD ) != 0 )
	{
		return Convert_Bitfield_Internal( value, entry_name );
	}
	else
	{
		return Convert_Internal( value, entry_name );
	}
}

/**********************************************************************************************************************
	CConvertibleEnum::Register_Entry -- registers a string and integer value pair as convertible

		entry_name -- corresponding string value
		value -- integer to convert to/from

**********************************************************************************************************************/
void CConvertibleEnum::Register_Entry( const std::string &entry_name, uint64_t value )
{
	std::string upper_entry_name;
	NStringUtils::To_Upper_Case( entry_name, upper_entry_name );

	auto name_iter = NameToValueTable.find( upper_entry_name );
	FATAL_ASSERT( name_iter == NameToValueTable.cend() );

	auto value_iter = ValueToNameTable.find( value );
	FATAL_ASSERT( value_iter == ValueToNameTable.cend() );

	NameToValueTable[ upper_entry_name ] = value;
	ValueToNameTable[ value ] = upper_entry_name;
}

/**********************************************************************************************************************
	CConvertibleEnum::Convert_Internal -- internal function to convert from a string to an integer value

		entry_name -- string value to convert from
		output_value -- output parameter for the integer to convert to

		Returns: success/failure

**********************************************************************************************************************/
bool CConvertibleEnum::Convert_Internal( const std::string &entry_name, uint64_t &output_value ) const
{
	std::string upper_entry_name;
	NStringUtils::To_Upper_Case( entry_name, upper_entry_name );

	auto iter = NameToValueTable.find( upper_entry_name );
	if ( iter == NameToValueTable.cend() )
	{
		return false;
	}

	output_value = iter->second;
	return true;
}

/**********************************************************************************************************************
	CConvertibleEnum::Convert_Internal -- internal function to convert from an integer value to a string

		value -- integer to convert from
		entry_name -- output parameter for the string value 

		Returns: success/failure

**********************************************************************************************************************/
bool CConvertibleEnum::Convert_Internal( uint64_t value, std::string &entry_name ) const
{
	auto iter = ValueToNameTable.find( value );
	if ( iter == ValueToNameTable.cend() )
	{
		return false;
	}

	entry_name = iter->second;
	return true;
}

/**********************************************************************************************************************
	Skip_Separators -- scans from a position within a raw character sequence until the string end is found or a non
		separator character ( space, tab, and | ) is found

		string_buffer -- raw character sequence to scan
		index -- position within the string to start the can from

		Returns: position of the first non-separator character or the end of the string

**********************************************************************************************************************/
static uint32_t Skip_Separators( const char *string_buffer, uint32_t index )
{
	char current_char = *( string_buffer + index );
	while ( current_char != 0 && ( current_char == ' ' || current_char == '\t' || current_char == '|' ) )
	{
		current_char = *( string_buffer + ++index );
	}

	return index;
}

/**********************************************************************************************************************
	Skip_Non_Separators -- scans from a position within a raw character sequence until the string end is found or a 
		separator character ( space, tab, and | ) is found

		string_buffer -- raw character sequence to scan
		index -- position within the string to start the can from

		Returns: position of the first separator character or the end of the string

**********************************************************************************************************************/
static uint32_t Skip_Non_Separators( const char *string_buffer, uint32_t index )
{
	char current_char = *( string_buffer + index );
	while ( current_char != 0 && ( current_char != ' ' && current_char != '\t' && current_char != '|' ) )
	{
		current_char = *( string_buffer + ++index );
	}

	return index;
}

/**********************************************************************************************************************
	CConvertibleEnum::Convert_Bitfield_Internal -- internal function to convert from a bitfield string to an integer value

		mask_name -- bitfield string value to convert from
		output_value -- output parameter for the integer to convert to

		Returns: success/failure

**********************************************************************************************************************/
bool CConvertibleEnum::Convert_Bitfield_Internal( const std::string &mask_name, uint64_t &output_value ) const
{
	const char *raw_characters = mask_name.c_str();
	output_value = 0;

	uint32_t entry_start = Skip_Separators( raw_characters, 0 );
	while ( entry_start < mask_name.size() )
	{
		uint32_t entry_end = Skip_Non_Separators( raw_characters, entry_start );

		std::string entry( raw_characters + entry_start, raw_characters + entry_end );
		uint64_t entry_value = 0;

		if ( !Convert_Internal( entry, entry_value ) )
		{
			return false;
		}

		output_value |= entry_value;

		entry_start = Skip_Separators( raw_characters, entry_end );
	}

	return true;
}

/**********************************************************************************************************************
	CConvertibleEnum::Convert_Bitfield_Internal -- internal function to convert from a bitfield integer value to a string

		value -- bitfield integer to convert from
		entry_name -- output parameter for the string value 

		Returns: success/failure

**********************************************************************************************************************/
bool CConvertibleEnum::Convert_Bitfield_Internal( uint64_t value, std::string &mask_name ) const
{
	if ( value == 0 )
	{
		return Convert_Internal( value, mask_name );
	}

	mask_name = "";
	uint64_t value_iterator = value;
	bool first_entry = true;

	for ( uint32_t bit_index = 0; value_iterator > 0; bit_index++, value_iterator >>= 1 )
	{
		if ( ( value_iterator & 0x01 ) != 0 )
		{
			uint64_t bit_value = 1UL << bit_index;

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

CEnumConverter::EnumTableType CEnumConverter::Enums;
std::unordered_map< std::string, CConvertibleEnum * > CEnumConverter::EnumsByName;

/**********************************************************************************************************************
	CEnumConverter::Cleanup -- deletes the enum objects used to perform conversions

**********************************************************************************************************************/
void CEnumConverter::Cleanup( void )
{
	std::for_each( Enums.begin(), Enums.end(), []( const EnumTableType::value_type &val ){ SAFE_DELETE( val.second ); } );
	Enums.clear();
	EnumsByName.clear();
}

/**********************************************************************************************************************
	CEnumConverter::Register_Enum_Internal -- instantiates an enum conversion object for the supplied enum

		enum_type_id -- type info of the enum type
		enum_name -- name of the enum to register conversion entries for
		properties -- any special properties that the enum has (bitfield)

**********************************************************************************************************************/
void CEnumConverter::Register_Enum_Internal( const Loki::TypeInfo &enum_type_info, const std::string &enum_name, EConvertibleEnumProperties properties )
{
	FATAL_ASSERT( Enums.find( enum_type_info ) == Enums.cend() );

	std::string upper_enum_name;
	NStringUtils::To_Upper_Case( enum_name, upper_enum_name );
	FATAL_ASSERT( EnumsByName.find( upper_enum_name ) == EnumsByName.cend() );

	CConvertibleEnum *enum_object = new CConvertibleEnum( enum_name, properties );
	Enums[ enum_type_info ] = enum_object;
	EnumsByName[ upper_enum_name ] = enum_object;
}

/**********************************************************************************************************************
	CEnumConverter::Register_Enum_Entry_Internal -- registers a string<->integer conversion pair within an enum

		enum_type_id -- type info of the enum to register a conversion entry for
		entry_name -- string representation of the enum entry value
		entry_value -- integer value of the enum entry

**********************************************************************************************************************/		
void CEnumConverter::Register_Enum_Entry_Internal( const Loki::TypeInfo &enum_type_info, const std::string &entry_name, uint64_t entry_value )
{
	CConvertibleEnum *enum_object = Find_Enum( enum_type_info );
	FATAL_ASSERT( enum_object != nullptr );

	enum_object->Register_Entry( entry_name, entry_value );
}

/**********************************************************************************************************************
	CEnumConverter::Find_Enum -- searches for the conversion object for the named enum

		enum_type_id -- type info for the enum to find

		Returns: the conversion object for the name enum, or null

**********************************************************************************************************************/	
CConvertibleEnum *CEnumConverter::Find_Enum( const Loki::TypeInfo &enum_type_info )
{
	auto iter = Enums.find( enum_type_info );
	if ( iter == Enums.end() )
	{
		return nullptr;
	}

	return iter->second;
}

/**********************************************************************************************************************
	CEnumConverter::Find_Enum -- searches for the conversion object for the named enum

		enum_name -- name of the enum to find

		Returns: the conversion object for the name enum, or null

**********************************************************************************************************************/	
CConvertibleEnum *CEnumConverter::Find_Enum( const std::string &enum_name )
{
	std::string upper_enum_name;
	NStringUtils::To_Upper_Case( enum_name, upper_enum_name );

	auto iter = EnumsByName.find( upper_enum_name );
	if ( iter == EnumsByName.end() )
	{
		return nullptr;
	}

	return iter->second;
}

/**********************************************************************************************************************
	CEnumConverter::Convert -- converts from a string to an integer for the supplied enum

		enum_type_id -- type info for the enum this is a conversion operation for
		entry_name -- string value to convert from
		output_value -- output parameter for the corresponding integer value

**********************************************************************************************************************/	
bool CEnumConverter::Convert( const Loki::TypeInfo &enum_type_info, const std::wstring &entry_name, uint64_t &output_value )
{
	std::string usable_entry_name;
	NStringUtils::WideString_To_String( entry_name, usable_entry_name );

	return Convert_Internal( enum_type_info, usable_entry_name, output_value );
}

/**********************************************************************************************************************
	CEnumConverter::Convert -- converts from a string to an integer for the supplied enum

		enum_type_id -- type info for the enum this is a conversion operation for
		entry_name -- string value to convert from
		output_value -- output parameter for the corresponding integer value

**********************************************************************************************************************/	
bool CEnumConverter::Convert_Internal( const Loki::TypeInfo &enum_type_info, const std::string &entry_name, uint64_t &output_value )
{
	CConvertibleEnum *enum_object = Find_Enum( enum_type_info );
	if ( enum_object == nullptr )
	{
		return false;
	}

	return enum_object->Convert( entry_name, output_value );
}

/**********************************************************************************************************************
	CEnumConverter::Convert_Internal -- converts from an integer to a string for the supplied enum

		enum_name -- name of the enum this is a conversion operation for
		value -- integer value to convert from
		entry_name -- output parameter for the corresponding string value

**********************************************************************************************************************/	
bool CEnumConverter::Convert_Internal( const Loki::TypeInfo &enum_type_info, uint64_t value, std::string &entry_name )
{
	CConvertibleEnum *enum_object = Find_Enum( enum_type_info );
	if ( enum_object == nullptr )
	{
		return false;
	}

	return enum_object->Convert( value, entry_name );
}

/**********************************************************************************************************************
	CEnumConverter::Convert_Internal -- converts from an integer to a wstring for the supplied enum

		enum_name -- name of the enum this is a conversion operation for
		value -- integer value to convert from
		entry_name -- output parameter for the corresponding string value

**********************************************************************************************************************/	
bool CEnumConverter::Convert_Internal( const Loki::TypeInfo &enum_type_info, uint64_t value, std::wstring &entry_name )
{
	CConvertibleEnum *enum_object = Find_Enum( enum_type_info );
	if ( enum_object == nullptr )
	{
		return false;
	}

	std::string narrow_entry_name;
	bool success = enum_object->Convert( value, narrow_entry_name );
	if ( success )
	{
		NStringUtils::String_To_WideString( narrow_entry_name, entry_name );
	}

	return success;
}

/**********************************************************************************************************************
	CEnumConverter::Convert -- converts from a string to an integer for the supplied enum

		enum_name -- name of the enum this is a conversion operation for
		entry_name -- string value to convert from
		output_value -- output parameter for the corresponding integer value

**********************************************************************************************************************/	
bool CEnumConverter::Convert( const std::string &enum_name, const std::wstring &entry_name, uint64_t &output_value )
{
	CConvertibleEnum *enum_object = Find_Enum( enum_name );
	if ( enum_object == nullptr )
	{
		return false;
	}

	std::string usable_entry_name;
	NStringUtils::WideString_To_String( entry_name, usable_entry_name );

	return enum_object->Convert( usable_entry_name, output_value );
}
