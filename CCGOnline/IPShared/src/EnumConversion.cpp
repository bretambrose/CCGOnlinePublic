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

#include <IPShared/EnumConversion.h>

#include <IPCore/Debug/DebugAssert.h>
#include <IPCore/Utils/EnumUtils.h>
#include <IPCore/Utils/StringUtils.h>
#include <loki/LokiTypeInfo.h>

#include <algorithm>

namespace IP
{
namespace Enum
{

// A simple, hidden class that contains all the conversion information for a single enum
class CConvertibleEnum 
{
	public:

		// Construction
		CConvertibleEnum( const IP::String &name, EConvertibleEnumProperties properties );

		// Accessors
		const IP::String &Get_Name( void ) const { return Name; }
		EConvertibleEnumProperties Get_Properties( void ) const { return Properties; }

		// Conversion registration
		void Register_Entry( const IP::String &entry_name, uint64_t value );

		// Conversion
		bool Convert( const IP::String &entry_name, uint64_t &output_value ) const;
		bool Convert( uint64_t output_value, IP::String &entry_name ) const;

	private:

		bool Convert_Internal( const IP::String &entry_name, uint64_t &output_value ) const;
		bool Convert_Internal( uint64_t value, IP::String &entry_name ) const;

		bool Convert_Bitfield_Internal( const IP::String &mask_name, uint64_t &output_value ) const;
		bool Convert_Bitfield_Internal( uint64_t value, IP::String &mask_name ) const;

		// Data
		IP::String Name;

		EConvertibleEnumProperties Properties;

		IP::UnorderedMap< IP::String, uint64_t > NameToValueTable;
		IP::UnorderedMap< uint64_t, IP::String > ValueToNameTable;
};


CConvertibleEnum::CConvertibleEnum( const IP::String &name, EConvertibleEnumProperties properties ) :
	Name( name ),
	Properties( properties ),
	NameToValueTable(),
	ValueToNameTable()
{
}


bool CConvertibleEnum::Convert( const IP::String &entry_name, uint64_t &output_value ) const
{
	if ( Is_An_Enum_Flag_Set( Properties, EConvertibleEnumProperties::CEP_BITFIELD ) )
	{
		return Convert_Bitfield_Internal( entry_name, output_value );
	}
	else
	{
		return Convert_Internal( entry_name, output_value );
	}
}


bool CConvertibleEnum::Convert( uint64_t value, IP::String &entry_name ) const
{
	if ( Is_An_Enum_Flag_Set( Properties, EConvertibleEnumProperties::CEP_BITFIELD ) )
	{
		return Convert_Bitfield_Internal( value, entry_name );
	}
	else
	{
		return Convert_Internal( value, entry_name );
	}
}


void CConvertibleEnum::Register_Entry( const IP::String &entry_name, uint64_t value )
{
	IP::String upper_entry_name;
	IP::StringUtils::To_Upper_Case( entry_name, upper_entry_name );

	auto name_iter = NameToValueTable.find( upper_entry_name );
	FATAL_ASSERT( name_iter == NameToValueTable.cend() );

	auto value_iter = ValueToNameTable.find( value );
	FATAL_ASSERT( value_iter == ValueToNameTable.cend() );

	NameToValueTable[ upper_entry_name ] = value;
	ValueToNameTable[ value ] = upper_entry_name;
}


bool CConvertibleEnum::Convert_Internal( const IP::String &entry_name, uint64_t &output_value ) const
{
	IP::String upper_entry_name;
	IP::StringUtils::To_Upper_Case( entry_name, upper_entry_name );

	auto iter = NameToValueTable.find( upper_entry_name );
	if ( iter == NameToValueTable.cend() )
	{
		return false;
	}

	output_value = iter->second;
	return true;
}


bool CConvertibleEnum::Convert_Internal( uint64_t value, IP::String &entry_name ) const
{
	auto iter = ValueToNameTable.find( value );
	if ( iter == ValueToNameTable.cend() )
	{
		return false;
	}

	entry_name = iter->second;
	return true;
}


static uint32_t Skip_Separators( const char *string_buffer, uint32_t index )
{
	char current_char = *( string_buffer + index );
	while ( current_char != 0 && ( current_char == ' ' || current_char == '\t' || current_char == '|' ) )
	{
		current_char = *( string_buffer + ++index );
	}

	return index;
}


static uint32_t Skip_Non_Separators( const char *string_buffer, uint32_t index )
{
	char current_char = *( string_buffer + index );
	while ( current_char != 0 && ( current_char != ' ' && current_char != '\t' && current_char != '|' ) )
	{
		current_char = *( string_buffer + ++index );
	}

	return index;
}


bool CConvertibleEnum::Convert_Bitfield_Internal( const IP::String &mask_name, uint64_t &output_value ) const
{
	const char *raw_characters = mask_name.c_str();
	output_value = 0;

	uint32_t entry_start = Skip_Separators( raw_characters, 0 );
	while ( entry_start < mask_name.size() )
	{
		uint32_t entry_end = Skip_Non_Separators( raw_characters, entry_start );

		IP::String entry( raw_characters + entry_start, raw_characters + entry_end );
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


bool CConvertibleEnum::Convert_Bitfield_Internal( uint64_t value, IP::String &mask_name ) const
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

			IP::String entry_name;
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
IP::UnorderedMap< IP::String, CConvertibleEnum * > CEnumConverter::EnumsByName;


void CEnumConverter::Cleanup( void )
{
	std::for_each( Enums.begin(), Enums.end(), []( const EnumTableType::value_type &val ){ IP::Delete( val.second ); } );
	Enums.clear();
	EnumsByName.clear();
}


void CEnumConverter::Register_Enum_Internal( const Loki::TypeInfo &enum_type_info, const IP::String &enum_name, EConvertibleEnumProperties properties )
{
	FATAL_ASSERT( Enums.find( enum_type_info ) == Enums.cend() );

	IP::String upper_enum_name;
	IP::StringUtils::To_Upper_Case( enum_name, upper_enum_name );
	FATAL_ASSERT( EnumsByName.find( upper_enum_name ) == EnumsByName.cend() );

	CConvertibleEnum *enum_object = IP::New< CConvertibleEnum >( MEMORY_TAG, enum_name, properties );
	Enums[ enum_type_info ] = enum_object;
	EnumsByName[ upper_enum_name ] = enum_object;
}

		
void CEnumConverter::Register_Enum_Entry_Internal( const Loki::TypeInfo &enum_type_info, const IP::String &entry_name, uint64_t entry_value )
{
	CConvertibleEnum *enum_object = Find_Enum( enum_type_info );
	FATAL_ASSERT( enum_object != nullptr );

	enum_object->Register_Entry( entry_name, entry_value );
}


CConvertibleEnum *CEnumConverter::Find_Enum( const Loki::TypeInfo &enum_type_info )
{
	auto iter = Enums.find( enum_type_info );
	if ( iter == Enums.end() )
	{
		return nullptr;
	}

	return iter->second;
}


CConvertibleEnum *CEnumConverter::Find_Enum( const IP::String &enum_name )
{
	IP::String upper_enum_name;
	IP::StringUtils::To_Upper_Case( enum_name, upper_enum_name );

	auto iter = EnumsByName.find( upper_enum_name );
	if ( iter == EnumsByName.end() )
	{
		return nullptr;
	}

	return iter->second;
}


bool CEnumConverter::Convert( const Loki::TypeInfo &enum_type_info, const IP::String &entry_name, uint64_t &output_value )
{
	return Convert_Internal( enum_type_info, entry_name, output_value );
}


bool CEnumConverter::Convert_Internal( const Loki::TypeInfo &enum_type_info, const IP::String &entry_name, uint64_t &output_value )
{
	CConvertibleEnum *enum_object = Find_Enum( enum_type_info );
	if ( enum_object == nullptr )
	{
		return false;
	}

	return enum_object->Convert( entry_name, output_value );
}


bool CEnumConverter::Convert_Internal( const Loki::TypeInfo &enum_type_info, uint64_t value, IP::String &entry_name )
{
	CConvertibleEnum *enum_object = Find_Enum( enum_type_info );
	if ( enum_object == nullptr )
	{
		return false;
	}

	return enum_object->Convert( value, entry_name );
}

bool CEnumConverter::Convert( const IP::String &enum_name, const IP::String &entry_name, uint64_t &output_value )
{
	CConvertibleEnum *enum_object = Find_Enum( enum_name );
	if ( enum_object == nullptr )
	{
		return false;
	}

	return enum_object->Convert( entry_name, output_value );
}

} // namespace Enum
} // namespace IP