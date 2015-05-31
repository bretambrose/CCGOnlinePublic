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

#ifndef ENUM_CONVERSION_H
#define ENUM_CONVERSION_H

enum EConvertibleEnumProperties
{
	CEP_NONE						= 0,
	CEP_BITFIELD				= 1 << 0,
};

class CConvertibleEnum;

// A simple static class with registration and conversion functions for enums
// Registration should be done at startup time before any concurrency is established.
// After that, multiple threads should be able to safely call Convert functions without worry of
// conflice
class CEnumConverter
{
	public:

		static void Cleanup( void );

		// Registration
		template < typename T >
		static void Register_Enum( const std::string &enum_name, EConvertibleEnumProperties properties )
		{
			Register_Enum_Internal( Loki::TypeInfo( typeid( T ) ), enum_name, properties );
		}

		template < typename T >
		static void Register_Enum_Entry( const std::string &entry_name, T entry_value )
		{
			Register_Enum_Entry_Internal( Loki::TypeInfo( typeid( T ) ), entry_name, static_cast< uint64_t >( entry_value ) );
		}

		// Conversion
		template < typename T >
		static bool Convert( const std::string &entry_name, T &output_value )
		{
			uint64_t converted_value = 0;
			bool success = Convert_Internal( Loki::TypeInfo( typeid( T ) ), entry_name, converted_value );
			output_value = static_cast< T >( converted_value );

			return success;
		}

		template < typename T >
		static bool Convert( const std::wstring &entry_name, T &output_value )
		{
			std::string usable_entry_name;
			NStringUtils::WideString_To_String( entry_name, usable_entry_name );

			return Convert( usable_entry_name, output_value );
		}

		template < typename T >
		static bool Convert( T output_value, std::string &entry_name )
		{
			uint64_t converted_value = static_cast< uint64_t >( output_value );
			return Convert_Internal( Loki::TypeInfo( typeid( T ) ), converted_value, entry_name );
		}

		template < typename T >
		static bool Convert( T output_value, std::wstring &entry_name )
		{
			uint64_t converted_value = static_cast< uint64_t >( output_value );
			return Convert_Internal( Loki::TypeInfo( typeid( T ) ), converted_value, entry_name );
		}

		static bool Convert( const std::string &enum_name, const std::wstring &entry_name, uint64_t &output_value );
		static bool Convert( const Loki::TypeInfo &enum_type_info, const std::wstring &entry_name, uint64_t &output_value );

	private:

		static CConvertibleEnum *Find_Enum( const Loki::TypeInfo &enum_type_info);
		static CConvertibleEnum *Find_Enum( const std::string &enum_name );

		static void Register_Enum_Internal( const Loki::TypeInfo &enum_type_info, const std::string &upper_enum_name, EConvertibleEnumProperties properties );
		static void Register_Enum_Entry_Internal( const Loki::TypeInfo &enum_type_info, const std::string &entry_name, uint64_t entry_value );

		static bool Convert_Internal( const Loki::TypeInfo &enum_type_info, uint64_t value, std::string &entry_name );
		static bool Convert_Internal( const Loki::TypeInfo &enum_type_info, uint64_t value, std::wstring &entry_name );
		static bool Convert_Internal( const Loki::TypeInfo &enum_type_info, const std::string &entry_name, uint64_t &output_value );

		typedef std::unordered_map< Loki::TypeInfo, CConvertibleEnum *, STypeInfoContainerHelper > EnumTableType;

		static EnumTableType Enums;
		static std::unordered_map< std::string, CConvertibleEnum * > EnumsByName;
};

#endif // ENUM_CONVERSION_H