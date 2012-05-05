/**********************************************************************************************************************

	ODBCParameters.h
		A component defining various ODBC specific parameter types to be used as inputs to stored procedures

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

#include "Database/Interfaces/DatabaseVariableInterface.h"
#include "Database/DatabaseTypes.h"

template < typename T >
EDatabaseVariableValueType Get_ODBC_Value_Type( const T & /*dummy*/ )
{
	return DVVT_INVALID;
}

EDatabaseVariableValueType Get_ODBC_Value_Type( const int32 & /*dummy*/ ) { return DVVT_INT32; }
EDatabaseVariableValueType Get_ODBC_Value_Type( const uint32 & /*dummy*/ ) { return DVVT_UINT32; }
EDatabaseVariableValueType Get_ODBC_Value_Type( const int64 & /*dummy*/ ) { return DVVT_INT64; }
EDatabaseVariableValueType Get_ODBC_Value_Type( const uint64 & /*dummy*/ ) { return DVVT_UINT64; }
EDatabaseVariableValueType Get_ODBC_Value_Type( const float & /*dummy*/ ) { return DVVT_FLOAT; }
EDatabaseVariableValueType Get_ODBC_Value_Type( const double & /*dummy*/ ) { return DVVT_DOUBLE; }

template < typename T, EDatabaseVariableType PT >
class TODBCScalarVariable : public IDatabaseVariable
{
	public:

		typedef IDatabaseVariable BASECLASS;

		TODBCScalarVariable( void ) :
			Value( static_cast< T >( 0 ) ),
			Indicator( SQL_NULL_DATA )
		{}

		TODBCScalarVariable( const TODBCScalarVariable< T, PT > &rhs ) :
			Value( rhs.Value ),
			Indicator( rhs.Indicator )
		{}

		explicit TODBCScalarVariable( const T &value ) :
			Value( value ),
			Indicator( 0 )
		{}

		virtual ~TODBCScalarVariable() {}

		// Baseclass interface
		virtual EDatabaseVariableValueType Get_Value_Type( void ) const { return Get_ODBC_Value_Type( Value ); }
		virtual EDatabaseVariableType Get_Parameter_Type( void ) const { return PT; }
		virtual uint32 Get_Decimals( void ) const { return 0; }
		virtual void *Get_Value_Address( void ) { return &Value; }
		virtual void *Get_Auxiliary_Address( void ) { return &Indicator; }
		virtual uint32 Get_Value_Size( void ) const { return 0; }
		virtual uint32 Get_Value_Buffer_Size( void ) const { return 0; }

		// Additional interface
		const T &Get_Value( void ) const { return Value; }
		void Set_Value( const T &value ) { Value = value; Indicator = 0; }

		bool Is_Null( void ) const { return Indicator == SQL_NULL_DATA; }
		void Set_Null( void ) { Indicator = SQL_NULL_DATA; }

	private:

		T Value;
		SQLLEN Indicator;
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

typedef TODBCScalarVariable< int32, DVT_INPUT >					DBInt32In;
typedef TODBCScalarVariable< int32, DVT_INPUT_OUTPUT >		DBInt32InOut;
typedef TODBCScalarVariable< int32, DVT_OUTPUT >				DBInt32Out;

typedef TODBCScalarVariable< uint32, DVT_INPUT >				DBUInt32In;
typedef TODBCScalarVariable< uint32, DVT_INPUT_OUTPUT >		DBUInt32InOut;
typedef TODBCScalarVariable< uint32, DVT_OUTPUT >				DBUInt32Out;

typedef TODBCScalarVariable< int64, DVT_INPUT >					DBInt64In;
typedef TODBCScalarVariable< int64, DVT_INPUT_OUTPUT >		DBInt64InOut;
typedef TODBCScalarVariable< int64, DVT_OUTPUT >				DBInt64Out;

typedef TODBCScalarVariable< uint64, DVT_INPUT >				DBUInt64In;
typedef TODBCScalarVariable< uint64, DVT_INPUT_OUTPUT >		DBUInt64InOut;
typedef TODBCScalarVariable< uint64, DVT_OUTPUT >				DBUInt64Out;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

template < uint32 BUFFER_LENGTH, EDatabaseVariableType PT = DVT_INPUT >
class DBString : public IDatabaseVariable
{
	public:

		typedef IDatabaseVariable BASECLASS;

		DBString( void ) :
			Indicator( SQL_NULL_DATA )
		{
		}

		DBString( const DBString< BUFFER_LENGTH, PT > &rhs ) :
			Indicator( rhs.Indicator )
		{
			memcpy( Buffer, rhs.Buffer, ( BUFFER_LENGTH + 1 ) * sizeof( char ) );
		}

		explicit DBString( const std::string &buffer ) :
			Indicator( 0 )
		{
			Set_Value( buffer.c_str() );
		}

		explicit DBString( const char *buffer ) :
			Indicator( 0 )
		{
			Set_Value( buffer );
		}

		virtual ~DBString() {}

		// Baseclass interface
		virtual EDatabaseVariableValueType Get_Value_Type( void ) const { return DVVT_STRING; }
		virtual EDatabaseVariableType Get_Parameter_Type( void ) const { return PT; }
		virtual uint32 Get_Decimals( void ) const { return 0; }
		virtual void *Get_Value_Address( void ) { return static_cast< char * >( Buffer ); }
		virtual void *Get_Auxiliary_Address( void ) { return &Indicator; }
		virtual uint32 Get_Value_Size( void ) const { return BUFFER_LENGTH + 1; }
		virtual uint32 Get_Value_Buffer_Size( void ) const { return BUFFER_LENGTH + 1; }

		// Additional interface
		const char *Get_Buffer( void ) const { return Buffer; }

		void Copy_Into( std::string &dest )
		{
			FATAL_ASSERT( !Is_Null() );

			dest.assign( Buffer );
		}

		void Set_Value( const char *buffer ) 
		{ 
			if ( buffer != nullptr )
			{
				size_t length = strlen( buffer );
				if ( length > BUFFER_LENGTH )
				{
					length = BUFFER_LENGTH;
				}

				memcpy( Buffer, buffer, length * sizeof( char ) );
				Buffer[ length ] = 0;
			}

			Indicator = ( buffer == nullptr ) ? SQL_NULL_DATA : SQL_NTS;
		}

		bool Is_Null( void ) const { return Indicator == SQL_NULL_DATA; }

	private:

		char Buffer[ BUFFER_LENGTH + 1 ];
		SQLLEN Indicator;

};

template < uint32 BUFFER_LENGTH, EDatabaseVariableType PT = DVT_INPUT >
class DBWString : public IDatabaseVariable
{
	public:

		typedef IDatabaseVariable BASECLASS;

		DBWString( void ) :
			Indicator( SQL_NULL_DATA )
		{
		}

		DBWString( const DBString< BUFFER_LENGTH, PT > &rhs ) :
			Indicator( rhs.Indicator )
		{
			memcpy( Buffer, rhs.Buffer, ( BUFFER_LENGTH + 1 ) * sizeof( wchar_t ) );
		}

		explicit DBWString( const std::wstring &buffer ) :
			Indicator( 0 )
		{
			Set_Value( buffer.c_str() );
		}

		explicit DBWString( const wchar_t *buffer ) :
			Indicator( 0 )
		{
			Set_Value( buffer );
		}

		virtual ~DBWString() {}

		// Baseclass interface
		virtual EDatabaseVariableValueType Get_Value_Type( void ) const { return DVVT_WSTRING; }
		virtual EDatabaseVariableType Get_Parameter_Type( void ) const { return PT; }
		virtual uint32 Get_Decimals( void ) const { return 0; }
		virtual void *Get_Value_Address( void ) { return static_cast< wchar_t * >( Buffer ); }
		virtual void *Get_Auxiliary_Address( void ) { return &Indicator; }
		virtual uint32 Get_Value_Size( void ) const { return BUFFER_LENGTH + 1; }
		virtual uint32 Get_Value_Buffer_Size( void ) const { return BUFFER_LENGTH + 1; }

		// Additional interface
		const wchar_t *Get_Buffer( void ) const { return Buffer; }

		void Copy_Into( std::wstring &dest )
		{
			FATAL_ASSERT( !Is_Null() );

			dest.assign( Buffer );
		}

		void Set_Value( const wchar_t *buffer ) 
		{ 
			if ( buffer != nullptr )
			{
				int32 length = wcslen( buffer );
				if ( length > BUFFER_LENGTH )
				{
					length = BUFFER_LENGTH;
				}

				memcpy( Buffer, buffer, length * sizeof( wchar_t ) );
				Buffer[ length ] = 0;
			}

			Indicator = ( buffer == nullptr ) ? SQL_NULL_DATA : SQL_NTS;
		}

		bool Is_Null( void ) const { return Indicator == SQL_NULL_DATA; }

	private:

		wchar_t Buffer[ BUFFER_LENGTH + 1 ];
		SQLLEN Indicator;

};

#endif // ODBC_PARAMETERS_H
