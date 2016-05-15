/**********************************************************************************************************************

	(c) Copyright 2012, Bret Ambrose (mailto:bretambrose@gmail.com).

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

#include <IPDatabase/IPDatabase.h>

#include <IPCore/Debug/DebugAssert.h>
#include <IPDatabase/Interfaces/DatabaseVariableInterface.h>
#include <IPDatabase/DatabaseTypes.h>
#include <IPDatabase/ODBCImplementation/ODBCParameterInsulation.h>
#include <IPCore/Utils/StringUtils.h>

namespace IP
{
namespace Db
{

template < typename T >
EDatabaseVariableValueType Get_ODBC_Value_Type( const T & /*dummy*/ )
{
	return DVVT_INVALID;
}

inline EDatabaseVariableValueType Get_ODBC_Value_Type( const int32_t & /*dummy*/ ) { return DVVT_INT32; }
inline EDatabaseVariableValueType Get_ODBC_Value_Type( const uint32_t & /*dummy*/ ) { return DVVT_UINT32; }
inline EDatabaseVariableValueType Get_ODBC_Value_Type( const int64_t & /*dummy*/ ) { return DVVT_INT64; }
inline EDatabaseVariableValueType Get_ODBC_Value_Type( const uint64_t & /*dummy*/ ) { return DVVT_UINT64; }
inline EDatabaseVariableValueType Get_ODBC_Value_Type( const float & /*dummy*/ ) { return DVVT_FLOAT; }
inline EDatabaseVariableValueType Get_ODBC_Value_Type( const double & /*dummy*/ ) { return DVVT_DOUBLE; }
inline EDatabaseVariableValueType Get_ODBC_Value_Type( const bool & /*dummy*/ ) { return DVVT_BOOLEAN; }

template < typename T >
class TODBCScalarVariableBase : public IDatabaseVariable
{
	public:

		using BASECLASS = IDatabaseVariable;

		TODBCScalarVariableBase( void ) :
			Value( static_cast< T >( 0 ) ),
			Indicator( IP_SQL_NULL_DATA )
		{}

		TODBCScalarVariableBase( const TODBCScalarVariableBase< T > &rhs ) :
			Value( rhs.Value ),
			Indicator( rhs.Indicator )
		{}

		explicit TODBCScalarVariableBase( const T &value ) :
			Value( value ),
			Indicator( 0 )
		{}

		virtual ~TODBCScalarVariableBase() {}

		TODBCScalarVariableBase< T > & operator =( const TODBCScalarVariableBase< T > &rhs )
		{
			if ( &rhs != this )
			{
				Value = rhs.Value;
				Indicator = rhs.Indicator;
			}

			return *this;
		}

		// Baseclass interface
		virtual EDatabaseVariableValueType Get_Value_Type( void ) const { return Get_ODBC_Value_Type( Value ); }
		virtual uint32_t Get_Decimals( void ) const { return 0; }
		virtual void *Get_Value_Address( void ) { return &Value; }
		virtual void *Get_Auxiliary_Address( void ) { return &Indicator; }
		virtual uint32_t Get_Value_Size( void ) const { return 0; }
		virtual uint32_t Get_Value_Buffer_Size( void ) const { return 0; }
		virtual bool Is_Null( void ) const { return Indicator == IP_SQL_NULL_DATA; }

		// Additional interface
		const T &Get_Value( void ) const { FATAL_ASSERT( !Is_Null() ); return Value; }
		void Set_Value( const T &value ) { Value = value; Indicator = 0; }

		void Set_Null( void ) { Indicator = IP_SQL_NULL_DATA; }

	private:

		T Value;
		IP_SQLLEN Indicator;
};


template < typename T, EDatabaseVariableType PT >
class TODBCScalarVariable : public TODBCScalarVariableBase< T >
{
	public:

		using BASECLASS = TODBCScalarVariableBase< T >;

		TODBCScalarVariable( void ) :
			BASECLASS()
		{}

		TODBCScalarVariable( const TODBCScalarVariable< T, PT > &rhs ) :
			BASECLASS( rhs )
		{}

		explicit TODBCScalarVariable( const T &value ) :
			BASECLASS( value )
		{}

		virtual ~TODBCScalarVariable() {}

		// Baseclass interface
		virtual EDatabaseVariableType Get_Parameter_Type( void ) const { return PT; }
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

using DBInt32In = TODBCScalarVariable< int32_t, DVT_INPUT >;
using DBInt32InOut = TODBCScalarVariable< int32_t, DVT_INPUT_OUTPUT >;
using DBInt32Out = TODBCScalarVariable< int32_t, DVT_OUTPUT >;

using DBUInt32In = TODBCScalarVariable< uint32_t, DVT_INPUT >;
using DBUInt32InOut = TODBCScalarVariable< uint32_t, DVT_INPUT_OUTPUT >;
using DBUInt32Out = TODBCScalarVariable< uint32_t, DVT_OUTPUT >;

using DBInt64In = TODBCScalarVariable< int64_t, DVT_INPUT >;
using DBInt64InOut = TODBCScalarVariable< int64_t, DVT_INPUT_OUTPUT >;
using DBInt64Out = TODBCScalarVariable< int64_t, DVT_OUTPUT >;

using DBUInt64In = TODBCScalarVariable< uint64_t, DVT_INPUT >;
using DBUInt64InOut = TODBCScalarVariable< uint64_t, DVT_INPUT_OUTPUT >;
using DBUInt64Out = TODBCScalarVariable< uint64_t, DVT_OUTPUT >;

using DBFloatIn = TODBCScalarVariable< float, DVT_INPUT >;
using DBFloatInOut = TODBCScalarVariable< float, DVT_INPUT_OUTPUT >;
using DBFloatOut = TODBCScalarVariable< float, DVT_OUTPUT >;

using DBDoubleIn = TODBCScalarVariable< double, DVT_INPUT >;
using DBDoubleInOut = TODBCScalarVariable< double, DVT_INPUT_OUTPUT >;
using DBDoubleOut = TODBCScalarVariable< double, DVT_OUTPUT >;

using DBBoolIn = TODBCScalarVariable< bool, DVT_INPUT >;
using DBBoolInOut = TODBCScalarVariable< bool, DVT_INPUT_OUTPUT >;
using DBBoolOut = TODBCScalarVariable< bool, DVT_OUTPUT >;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

IPDATABASE_API class IDatabaseString : public IDatabaseVariable
{
	public:

		using BASECLASS = IDatabaseVariable;

		IDatabaseString( void ) :
			BASECLASS()
		{}

		virtual ~IDatabaseString() {}

		virtual void To_String( IP::String &value ) const = 0;

};

template < uint32_t BUFFER_LENGTH >
class DBStringBase : public IDatabaseString
{
	public:

		using BASECLASS = IDatabaseString;

		DBStringBase( void ) :
			BASECLASS(),
			Indicator( IP_SQL_NULL_DATA )
		{
		}

		DBStringBase( const DBStringBase< BUFFER_LENGTH > &rhs ) :
			BASECLASS(),
			Indicator( rhs.Indicator )
		{
			memcpy( Buffer, rhs.Buffer, ( BUFFER_LENGTH + 1 ) * sizeof( char ) );
		}

		virtual ~DBStringBase() {}

		DBStringBase< BUFFER_LENGTH > & operator =( const DBStringBase< BUFFER_LENGTH > &rhs )
		{
			if ( &rhs != this )
			{
				memcpy( Buffer, rhs.Buffer, ( BUFFER_LENGTH + 1 ) * sizeof( char ) );
				Indicator = rhs.Indicator;
			}

			return *this;
		}

		// Baseclass interface
		virtual uint32_t Get_Decimals( void ) const { return 0; }
		virtual void *Get_Value_Address( void ) { return static_cast< void * >( Buffer ); }
		virtual void *Get_Auxiliary_Address( void ) { return &Indicator; }
		virtual uint32_t Get_Value_Size( void ) const { return BUFFER_LENGTH + 1; }
		virtual uint32_t Get_Value_Buffer_Size( void ) const { return BUFFER_LENGTH + 1; }
		virtual bool Is_Null( void ) const { return Indicator == IP_SQL_NULL_DATA; }

		// Additional interface
		const char *Get_Buffer( void ) const { return Buffer; }

	protected:

		void Set_Value_With_Length( const char *buffer, size_t length ) 
		{ 
			if ( buffer != nullptr )
			{
				if ( length > BUFFER_LENGTH )
				{
					length = BUFFER_LENGTH;
				}

				memcpy( Buffer, buffer, length * sizeof( char ) );
				Buffer[ length ] = 0;
			}

			Indicator = ( buffer == nullptr ) ? IP_SQL_NULL_DATA : IP_SQL_NTS;
		}

	private:

		char Buffer[ BUFFER_LENGTH + 1 ];
		IP_SQLLEN Indicator;

};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

template < uint32_t BUFFER_LENGTH, EDatabaseVariableType PT = DVT_INPUT >
class DBString : public DBStringBase< BUFFER_LENGTH >
{
	public:

		using BASECLASS = DBStringBase< BUFFER_LENGTH >;

		DBString( void ) :
			BASECLASS()
		{
		}

		// We could loosen this up with a templated constructor, but I don't see a compelling use case yet
		DBString( const DBString< BUFFER_LENGTH, PT > &rhs ) :
			BASECLASS( rhs )
		{
		}

		explicit DBString( const IP::String &buffer ) :
			BASECLASS()
		{
			Set_Value_With_Length( buffer.c_str(), buffer.size() );
		}

		explicit DBString( const char *buffer ) :
			BASECLASS()
		{
			Set_Value_With_Length( buffer, strlen( buffer ) );
		}

		virtual ~DBString() {}

		DBString< BUFFER_LENGTH, PT > & operator =( const DBString< BUFFER_LENGTH, PT > &rhs )
		{
			BASECLASS::operator =( rhs );

			return *this;
		}

		// Baseclass interface
		virtual EDatabaseVariableValueType Get_Value_Type( void ) const { return DVVT_STRING; }
		virtual EDatabaseVariableType Get_Parameter_Type( void ) const { return PT; }
		virtual void To_String( IP::String &value ) const
		{
			if ( Is_Null() )
			{
				value = "<NULL>";
			}
			else
			{
				value.assign( Get_Buffer() );
			}
		}

		void Copy_Into( IP::String &dest )
		{
			FATAL_ASSERT( !Is_Null() );

			dest.assign( Get_Buffer() );
		}

		void Set_Value( const IP::String &value )
		{
			Set_Value_With_Length( value.c_str(), value.size() );
		}
};

template < uint32_t BUFFER_LENGTH >
class DBStringIn : public DBString< BUFFER_LENGTH, DVT_INPUT >
{
	public:

		using BASECLASS = DBString< BUFFER_LENGTH, DVT_INPUT >;

		DBStringIn( void ) :
			BASECLASS()
		{
		}

		DBStringIn( const DBStringIn< BUFFER_LENGTH > &rhs ) :
			BASECLASS( rhs )
		{
		}

		explicit DBStringIn( const IP::String &buffer ) :
			BASECLASS( buffer )
		{
		}

		explicit DBStringIn( const char *buffer ) :
			BASECLASS( buffer )
		{
		}

		virtual ~DBStringIn() {}
};

template < uint32_t BUFFER_LENGTH >
class DBStringInOut : public DBString< BUFFER_LENGTH, DVT_INPUT_OUTPUT >
{
	public:

		using BASECLASS = DBString< BUFFER_LENGTH, DVT_INPUT_OUTPUT >;

		DBStringInOut( void ) :
			BASECLASS()
		{
		}

		DBStringInOut( const DBStringInOut< BUFFER_LENGTH > &rhs ) :
			BASECLASS( rhs )
		{
		}

		explicit DBStringInOut( const IP::String &buffer ) :
			BASECLASS( buffer )
		{
		}

		explicit DBStringInOut( const char *buffer ) :
			BASECLASS( buffer )
		{
		}

		virtual ~DBStringInOut() {}
};

template < uint32_t BUFFER_LENGTH >
class DBStringOut : public DBString< BUFFER_LENGTH, DVT_OUTPUT >
{
	public:

		using BASECLASS = DBString< BUFFER_LENGTH, DVT_OUTPUT >;

		DBStringOut( void ) :
			BASECLASS()
		{
		}

		DBStringOut( const DBStringOut< BUFFER_LENGTH > &rhs ) :
			BASECLASS( rhs )
		{
		}

		// Output params have no need for data-based constructors

		virtual ~DBStringOut() {}
};


} // namespace Db
} // namespace IP
