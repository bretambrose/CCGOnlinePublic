/**********************************************************************************************************************

	[Placeholder for eventual source license]

	PlatformMisc.cpp
		A component that wraps miscellaneous OS-specific logic that isn't well classifiable

	(c) Copyright 2011, Bret Ambrose.  All rights reserved.

**********************************************************************************************************************/

#include "stdafx.h"

#include "PlatformMisc.h"

/**********************************************************************************************************************
	Convert_GUID_To_Numeric -- reduces a 128 bit guid down to four 32 bit integers

		guid -- guid to reduce
		first32 -- output parameter for the first 32 bits
		second32 -- output parameter for the seconds 32 bits
		third32 -- output parameter for the third 32 bits
		fourth32 -- output parameter for the fourth 32 bits

**********************************************************************************************************************/
static void Convert_GUID_To_Numeric( const GUID &guid, uint32 &first32, uint32 &second32, uint32 &third32, uint32 &fourth32 )
{
	first32 = guid.Data1;
	
	second32 = guid.Data2;
	second32 <<= 16;
	second32 |= guid.Data3;

	third32 = 0;
	for ( uint32 i = 0; i < 4; i++ )
	{
		third32 |= guid.Data4[ i ];
		third32 <<= 8;
	}

	fourth32 = 0;
	for ( uint32 i = 4; i < 8; i++ )
	{
		fourth32 |= guid.Data4[ i ];
		fourth32 <<= 8;
	}
}

/**********************************************************************************************************************
	NPlatform::Get_Semi_Unique_ID -- builds a number likely to be unique without using pseudo rng

		Returns: a 32 bit unique key, used in non-critical hashing situations

**********************************************************************************************************************/
uint32 NPlatform::Get_Semi_Unique_ID( void )
{
	GUID guid;
	::CoCreateGuid( &guid ); 

	uint32 part1, part2, part3, part4;
	::Convert_GUID_To_Numeric( guid, part1, part2, part3, part4 );

	return part1 ^ part2 ^ part3 ^ part4;
}

/**********************************************************************************************************************
	NPlatform::Format_OS_Error_Message -- takes an os-specific error message and builds a descriptive string

		error_code -- os-specific error code

		Returns: a description of the error

**********************************************************************************************************************/
std::wstring NPlatform::Format_OS_Error_Message( uint32 error_code )
{
	wchar_t msg_buffer[ 1024 ];

	::FormatMessageW( FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
							NULL,
							error_code,
							MAKELANGID( LANG_NEUTRAL, SUBLANG_DEFAULT ),
							(LPTSTR) &msg_buffer,
							sizeof( msg_buffer ), 
							NULL );

	return std::wstring( msg_buffer );
}
