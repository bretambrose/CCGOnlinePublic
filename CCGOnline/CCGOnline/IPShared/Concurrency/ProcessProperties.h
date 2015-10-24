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

namespace IP
{
namespace Execution
{

/*
	A struct describing properties about an execution process.  This is an immutable data structure and it is not
	guaranteed to be unique across all processes.  The mode component may later be broken into flags, but that
	would imply a tri-value problem with matching (would probably need a separate match structure rather than
	sharing).
*/
struct SProcessProperties
{
	public:

		SProcessProperties( void ) :
			Value()
		{}

		SProcessProperties( const SProcessProperties &rhs ) :
			Value( rhs.Value )
		{}

		explicit SProcessProperties( uint16_t subject_part, uint16_t major_part = 1, uint16_t minor_part = 1, uint16_t mode_part = 1 ) :
			Value()
		{
			Value.Parts.Subject = subject_part;
			Value.Parts.Major = major_part;
			Value.Parts.Minor = minor_part;
			Value.Parts.Mode = mode_part;
		}

		// Accessors
		uint64_t Get_Raw_Value( void ) const { return Value.RawValue; }

		uint16_t Get_Subject( void ) const { return Value.Parts.Subject; }

		template< typename T >
		T Get_Subject_As( void ) const { return static_cast< T >( Value.Parts.Subject ); }

		uint16_t Get_Major_Part( void ) const { return Value.Parts.Major; }
		uint16_t Get_Minor_Part( void ) const { return Value.Parts.Minor; }
		uint16_t Get_Mode_Part( void ) const { return Value.Parts.Mode; }

		void Get_Parts( uint16_t &subject, uint16_t &major_part, uint16_t &minor_part, uint16_t &mode_part )
		{
			subject = Value.Parts.Subject;
			major_part = Value.Parts.Major;
			minor_part = Value.Parts.Minor;
			mode_part = Value.Parts.Mode;
		}

		template< typename T >
		void Get_Parts_As( T &subject, uint16_t &major_part, uint16_t &minor_part, uint16_t &mode_part )
		{
			subject = static_cast< T >( Value.Parts.Subject );
			major_part = Value.Parts.Major;
			minor_part = Value.Parts.Minor;
			mode_part = Value.Parts.Mode;
		}

		// Queries
		bool Is_Valid( void ) const
		{
			return Value.Parts.Subject != 0 && Value.Parts.Major != 0 && Value.Parts.Minor != 0 && Value.Parts.Mode != 0;
		}

		bool Matches( const SProcessProperties &properties ) const;

	private:

		struct SPropertyParts
		{
			unsigned Subject : 16;
			unsigned Major : 16;
			unsigned Minor : 16;
			unsigned Mode : 16;
		};

		union UPropertyValue
		{
			UPropertyValue( void ) :
				RawValue( 0 )
			{}

			uint64_t RawValue;
			SPropertyParts Parts;
		};

		UPropertyValue Value;
};

inline bool operator ==( const SProcessProperties &prop1, const SProcessProperties &prop2 )
{
	return prop1.Get_Raw_Value() == prop2.Get_Raw_Value();
}

inline bool operator !=( const SProcessProperties &prop1, const SProcessProperties &prop2 )
{
	return prop1.Get_Raw_Value() != prop2.Get_Raw_Value();
}

// Helper class that lets thread keys be used as keys in STL sets and maps
struct SProcessPropertiesContainerHelper
{
	public:

		enum {
			bucket_size = 4,  
			min_buckets = 8
		}; 

#ifdef X64
		size_t operator()( const SProcessProperties &prop_value ) const {
			return prop_value.Get_Raw_Value();
		}
#else
		size_t operator()( const SProcessProperties &prop_value ) const {
			uint64_t value = prop_value.Get_Raw_Value();

			return static_cast< size_t >( ( value & 0xFFFFFFFF ) ^ ( value >> 32 ) ); 
		}
#endif

		bool operator()( const SProcessProperties &left, const SProcessProperties &right ) const {
			return left.Get_Raw_Value() < right.Get_Raw_Value();
		}
};

} // namespace Execution
} // namespace IP