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

#ifndef ODBC_PARAMETERS_H
#define ODBC_PARAMETERS_H

#include "IPDatabase/Interfaces/DatabaseVariableInterface.h"
#include "IPDatabase/DatabaseTypes.h"
#include "ODBCParameterInsulation.h"
#include "IPPlatform/StringUtils.h"

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

		typedef IDatabaseVariable BASECLASS;

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

		typedef TODBCScalarVariableBase< T > BASECLASS;

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

typedef TODBCScalarVariable< int32_t, DVT_INPUT >					DBInt32In;
typedef TODBCScalarVariable< int32_t, DVT_INPUT_OUTPUT >		DBInt32InOut;
typedef TODBCScalarVariable< int32_t, DVT_OUTPUT >				DBInt32Out;

typedef TODBCScalarVariable< uint32_t, DVT_INPUT >				DBUInt32In;
typedef TODBCScalarVariable< uint32_t, DVT_INPUT_OUTPUT >		DBUInt32InOut;
typedef TODBCScalarVariable< uint32_t, DVT_OUTPUT >				DBUInt32Out;

typedef TODBCScalarVariable< int64_t, DVT_INPUT >					DBInt64In;
typedef TODBCScalarVariable< int64_t, DVT_INPUT_OUTPUT >		DBInt64InOut;
typedef TODBCScalarVariable< int64_t, DVT_OUTPUT >				DBInt64Out;

typedef TODBCScalarVariable< uint64_t, DVT_INPUT >				DBUInt64In;
typedef TODBCScalarVariable< uint64_t, DVT_INPUT_OUTPUT >		DBUInt64InOut;
typedef TODBCScalarVariable< uint64_t, DVT_OUTPUT >				DBUInt64Out;

typedef TODBCScalarVariable< float, DVT_INPUT >					DBFloatIn;
typedef TODBCScalarVariable< float, DVT_INPUT_OUTPUT >		DBFloatInOut;
typedef TODBCScalarVariable< float, DVT_OUTPUT >				DBFloatOut;

typedef TODBCScalarVariable< double, DVT_INPUT >				DBDoubleIn;
typedef TODBCScalarVariable< double, DVT_INPUT_OUTPUT >		DBDoubleInOut;
typedef TODBCScalarVariable< double, DVT_OUTPUT >				DBDoubleOut;

typedef TODBCScalarVariable< bool, DVT_INPUT >					DBBoolIn;
typedef TODBCScalarVariable< bool, DVT_INPUT_OUTPUT >			DBBoolInOut;
typedef TODBCScalarVariable< bool, DVT_OUTPUT >					DBBoolOut;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class IDatabaseString : public IDatabaseVariable
{
	public:

		typedef IDatabaseVariable BASECLASS;

		IDatabaseString( void ) :
			BASECLASS()
		{}

		virtual ~IDatabaseString() {}

		virtual void To_String( std::string &value ) const = 0;
		virtual void To_WString( std::wstring &value ) const = 0;

};

template < typename C, uint32_t BUFFER_LENGTH >
class DBStringBase : public IDatabaseString
{
	public:

		typedef IDatabaseString BASECLASS;

		DBStringBase( void ) :
			BASECLASS(),
			Indicator( IP_SQL_NULL_DATA )
		{
		}

		DBStringBase( const DBStringBase< C, BUFFER_LENGTH > &rhs ) :
			BASECLASS(),
			Indicator( rhs.Indicator )
		{
			memcpy( Buffer, rhs.Buffer, ( BUFFER_LENGTH + 1 ) * sizeof( C ) );
		}

		virtual ~DBStringBase() {}

		DBStringBase< C, BUFFER_LENGTH > & operator =( const DBStringBase< C, BUFFER_LENGTH > &rhs )
		{
			if ( &rhs != this )
			{
				memcpy( Buffer, rhs.Buffer, ( BUFFER_LENGTH + 1 ) * sizeof( C ) );
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
		const C *Get_Buffer( void ) const { return Buffer; }

	protected:

		void Set_Value_With_Length( const C *buffer, size_t length ) 
		{ 
			if ( buffer != nullptr )
			{
				if ( length > BUFFER_LENGTH )
				{
					length = BUFFER_LENGTH;
				}

				memcpy( Buffer, buffer, length * sizeof( C ) );
				Buffer[ length ] = 0;
			}

			Indicator = ( buffer == nullptr ) ? IP_SQL_NULL_DATA : IP_SQL_NTS;
		}

	private:

		C Buffer[ BUFFER_LENGTH + 1 ];
		IP_SQLLEN Indicator;

};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

template < uint32_t BUFFER_LENGTH, EDatabaseVariableType PT = DVT_INPUT >
class DBString : public DBStringBase< char, BUFFER_LENGTH >
{
	public:

		typedef DBStringBase< char, BUFFER_LENGTH > BASECLASS;

		DBString( void ) :
			BASECLASS()
		{
		}

		// We could loosen this up with a templated constructor, but I don't see a compelling use case yet
		DBString( const DBString< BUFFER_LENGTH, PT > &rhs ) :
			BASECLASS( rhs )
		{
		}

		explicit DBString( const std::string &buffer ) :
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
		virtual void To_String( std::string &value ) const
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

		virtual void To_WString( std::wstring &value ) const
		{
			std::string base_value;
			To_String( base_value );
			NStringUtils::String_To_WideString( base_value, value );
		}

		void Copy_Into( std::string &dest )
		{
			FATAL_ASSERT( !Is_Null() );

			dest.assign( Get_Buffer() );
		}

		void Set_Value( const std::string &value )
		{
			Set_Value_With_Length( value.c_str(), value.size() );
		}
};

template < uint32_t BUFFER_LENGTH >
class DBStringIn : public DBString< BUFFER_LENGTH, DVT_INPUT >
{
	public:

		typedef DBString< BUFFER_LENGTH, DVT_INPUT > BASECLASS;

		DBStringIn( void ) :
			BASECLASS()
		{
		}

		DBStringIn( const DBStringIn< BUFFER_LENGTH > &rhs ) :
			BASECLASS( rhs )
		{
		}

		explicit DBStringIn( const std::string &buffer ) :
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

		typedef DBString< BUFFER_LENGTH, DVT_INPUT_OUTPUT > BASECLASS;

		DBStringInOut( void ) :
			BASECLASS()
		{
		}

		DBStringInOut( const DBStringInOut< BUFFER_LENGTH > &rhs ) :
			BASECLASS( rhs )
		{
		}

		explicit DBStringInOut( const std::string &buffer ) :
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

		typedef DBString< BUFFER_LENGTH, DVT_OUTPUT > BASECLASS;

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

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

template < uint32_t BUFFER_LENGTH, EDatabaseVariableType PT = DVT_INPUT >
class DBWString : public DBStringBase< wchar_t, BUFFER_LENGTH >
{
	public:

		typedef DBStringBase< wchar_t, BUFFER_LENGTH > BASECLASS;

		DBWString( void ) :
			BASECLASS()
		{
		}

		// We could loosen this up with a templated constructor, but I don't see a compelling use case yet
		DBWString( const DBString< BUFFER_LENGTH, PT > &rhs ) :
			BASECLASS( rhs )
		{
		}

		explicit DBWString( const std::wstring &buffer ) :
			BASECLASS()
		{
			Set_Value_With_Length( buffer.c_str(), buffer.size() );
		}

		explicit DBWString( const wchar_t *buffer ) :
			BASECLASS
		{
			Set_Value_With_Length( buffer, wcslen( buffer ) );
		}

		virtual ~DBWString() {}

		DBWString< BUFFER_LENGTH, PT > & operator =( const DBWString< BUFFER_LENGTH, PT > &rhs )
		{
			BASECLASS::operator =( rhs );

			return *this;
		}

		// Baseclass interface
		virtual EDatabaseVariableValueType Get_Value_Type( void ) const { return DVVT_WSTRING; }
		virtual EDatabaseVariableType Get_Parameter_Type( void ) const { return PT; }

		virtual void To_String( std::string &value ) const
		{
			std::wstring base_value;
			To_WString( base_value );
			NStringUtils::WideString_To_String( base_value, value );
		}

		virtual void To_WString( std::wstring &value ) const
		{
			if ( Is_Null() )
			{
				value = L"<NULL>";
			}
			else
			{
				value.assign( Get_Buffer() );
			}
		}

		void Copy_Into( std::wstring &dest )
		{
			FATAL_ASSERT( !Is_Null() );

			dest.assign( Get_Buffer() );
		}

		void Set_Value( const std::wstring &value )
		{
			Set_Value_With_Length( value.c_str(), value.size() );
		}
};

template < uint32_t BUFFER_LENGTH >
class DBWStringIn : public DBWString< BUFFER_LENGTH, DVT_INPUT >
{
	public:

		typedef DBWString< BUFFER_LENGTH, DVT_INPUT > BASECLASS;

		DBWStringIn( void ) :
			BASECLASS()
		{
		}

		DBWStringIn( const DBWStringIn< BUFFER_LENGTH > &rhs ) :
			BASECLASS( rhs )
		{
		}

		explicit DBWStringIn( const std::wstring &buffer ) :
			BASECLASS( buffer )
		{
		}

		explicit DBWStringIn( const wchar_t *buffer ) :
			BASECLASS( buffer )
		{
		}

		virtual ~DBWStringIn() {}
};

template < uint32_t BUFFER_LENGTH >
class DBWStringInOut : public DBWString< BUFFER_LENGTH, DVT_INPUT_OUTPUT >
{
	public:

		typedef DBWString< BUFFER_LENGTH, DVT_INPUT_OUTPUT > BASECLASS;

		DBWStringInOut( void ) :
			BASECLASS()
		{
		}

		DBWStringInOut( const DBWStringInOut< BUFFER_LENGTH > &rhs ) :
			BASECLASS( rhs )
		{
		}

		explicit DBWStringInOut( const std::wstring &buffer ) :
			BASECLASS( buffer )
		{
		}

		explicit DBWStringInOut( const wchar_t *buffer ) :
			BASECLASS( buffer )
		{
		}

		virtual ~DBWStringInOut() {}
};

template < uint32_t BUFFER_LENGTH >
class DBWStringOut : public DBWString< BUFFER_LENGTH, DVT_OUTPUT >
{
	public:

		typedef DBWString< BUFFER_LENGTH, DVT_OUTPUT > BASECLASS;

		DBWStringOut( void ) :
			BASECLASS()
		{
		}

		DBWStringOut( const DBWStringOut< BUFFER_LENGTH > &rhs ) :
			BASECLASS( rhs )
		{
		}

		// Output params have no need for data-based constructors

		virtual ~DBWStringOut() {}
};

#endif // ODBC_PARAMETERS_H
