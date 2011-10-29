/**********************************************************************************************************************

	[Placeholder for eventual source license]

	ThreadKey.cpp
		A component definining the unique key that is bound to each thread task within the concurrency system

	(c) Copyright 2011, Bret Ambrose.  All rights reserved.

**********************************************************************************************************************/

#include "stdafx.h"

#include "ThreadKey.h"

#include "ThreadConstants.h"

/**********************************************************************************************************************
	SThreadKey::SThreadKey -- component-based constructor

		subject -- subject of the thread key
		major_index -- major sub key of the thread key
		minor_index -- minor sub key of the thread key
					
**********************************************************************************************************************/
SThreadKey::SThreadKey( EThreadSubject subject, uint16 major_index, uint16 minor_index ) :
	Key( Build_Key( subject, major_index, minor_index ) )
{
}

/**********************************************************************************************************************
	SThreadKey::Is_Valid -- checks a key for validity

		Returns: true if all the sub-components have been allocated and are valid, false otherwise
					
**********************************************************************************************************************/
bool SThreadKey::Is_Valid( void ) const 
{ 
	return Get_Major_Sub_Key() != INVALID_SUB_KEY && Get_Minor_Sub_Key() != INVALID_SUB_KEY && Get_Thread_Subject() != TS_INVALID; 
}

/**********************************************************************************************************************
	SThreadKey::Needs_Sub_Key_Allocation -- does this key need its major or minor sub keys allocated?

		Returns: true if allocation is needed, false otherwise
					
**********************************************************************************************************************/
bool SThreadKey::Needs_Sub_Key_Allocation( void ) const
{
	return Get_Thread_Subject() != TS_INVALID && ( Get_Minor_Sub_Key() == INVALID_SUB_KEY || Get_Major_Sub_Key() == INVALID_SUB_KEY );
}

/**********************************************************************************************************************
	SThreadKey::Matches -- does the supplied key match our implicit pattern?

		key -- key to check for a match against

		Returns: true if they key matches our pattern, false otherwise
					
**********************************************************************************************************************/
bool SThreadKey::Matches( const SThreadKey &key ) const
{
	// manager and log keys never match anything
	if ( key == MANAGER_THREAD_KEY || key == LOG_THREAD_KEY )
	{
		return false;
	}

	// if the subjects don't match and our pattern does not include a subject wildcard then there is no match
	if ( key.Get_Thread_Subject() != Get_Thread_Subject() && Get_Thread_Subject() != TS_ALL )
	{
		return false;
	}

	// if the major sub keys don't match and our pattern does not include a major subkey wildcard then there is no match
	if ( key.Get_Major_Sub_Key() != Get_Major_Sub_Key() && Get_Major_Sub_Key() != MAJOR_KEY_ALL )
	{
		return false;
	}

	// if the minor sub keys don't match and our pattern does not include a minor subkey wildcard then there is no match
	if ( key.Get_Minor_Sub_Key() != Get_Minor_Sub_Key() && Get_Minor_Sub_Key() != MINOR_KEY_ALL )
	{
		return false;
	}

	return true;
}

/**********************************************************************************************************************
	SThreadKey::Is_Unique -- does this key not contain any wildcard pattern matches?  Equivalent to Is_Valid

		Returns: true if they key contains no wildcards, false otherwise
					
**********************************************************************************************************************/
bool SThreadKey::Is_Unique( void ) const
{
	if ( Get_Thread_Subject() == TS_ALL )
	{
		return false;
	}

	if ( Get_Major_Sub_Key() == MAJOR_KEY_ALL )
	{
		return false;
	}

	if ( Get_Minor_Sub_Key() == MINOR_KEY_ALL )
	{
		return false;
	}

	return true;
}
