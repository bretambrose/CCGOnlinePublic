/**********************************************************************************************************************

	ThreadKeyManager.cpp
		A component definining a class that allocates and tracks thread keys

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

#include "ThreadKeyManager.h"

#include "ThreadKey.h"
#include "ThreadSubject.h"
#include "ThreadConstants.h"

/**********************************************************************************************************************
	CThreadKeyManager::CThreadKeyManager -- default constructor
					
**********************************************************************************************************************/
CThreadKeyManager::CThreadKeyManager( void ) :
	UsedMajorSubKeys(),
	UsedMinorSubKeys()
{
}

/**********************************************************************************************************************
	CThreadKeyManager::Fill_In_Thread_Key -- takes a thread key that may have uninstantiated sub-parts and allocates
		appropriate ids

		key -- the raw key that may need sub key allocation

		Returns: a fully instantiated, unique key
					
**********************************************************************************************************************/
SThreadKey CThreadKeyManager::Fill_In_Thread_Key( const SThreadKey &key )
{
	EThreadSubject subject = key.Get_Thread_Subject();
	FATAL_ASSERT( subject != TS_INVALID );

	SThreadKey thread_key = key;

	// Does the major sub key need allocation?
	if ( thread_key.Get_Major_Sub_Key() == INVALID_SUB_KEY )
	{
		FATAL_ASSERT( subject != TS_LOGGING && subject != TS_CONCURRENCY_MANAGER );
		thread_key.Set_Major_Sub_Key( Find_Next_Major_Sub_Key( thread_key.Get_Thread_Subject() ) );
	}

	// Does the minor sub key need allocation?
	if ( thread_key.Get_Minor_Sub_Key() == INVALID_SUB_KEY )
	{
		FATAL_ASSERT( subject != TS_LOGGING && subject != TS_CONCURRENCY_MANAGER );
		thread_key.Set_Minor_Sub_Key( Find_Next_Minor_Sub_Key( thread_key.Get_Thread_Subject(), thread_key.Get_Major_Sub_Key() ) );
	}

	return thread_key;
}

/**********************************************************************************************************************
	CThreadKeyManager::Add_Tracked_Thread_Key -- begins tracking a fully-instantiated key

		key -- the unique, fully instantiated key to track
					
**********************************************************************************************************************/
void CThreadKeyManager::Add_Tracked_Thread_Key( const SThreadKey &key )
{
	EThreadSubject subject = TS_INVALID;
	uint16 major_sub_key = INVALID_SUB_KEY;
	uint16 minor_sub_key = INVALID_SUB_KEY;

	key.Get_Key_Parts( subject, major_sub_key, minor_sub_key );

	// record the major key
	auto major_iter = UsedMajorSubKeys.find( subject );
	if ( major_iter == UsedMajorSubKeys.end() )
	{
		major_iter = UsedMajorSubKeys.insert( MajorSubKeyTableType::value_type( subject, std::set< uint16 >() ) ).first;
	}

	major_iter->second.insert( major_sub_key );

	// this is the <Subject, Major Sub Key> combination used a key for the minor key tracking table
	uint64 subject_major = SThreadKey::Build_Key( subject, major_sub_key, 0 );

	// record the minor key
	auto minor_iter = UsedMinorSubKeys.find( subject_major );
	if ( minor_iter == UsedMinorSubKeys.end() )
	{
		minor_iter = UsedMinorSubKeys.insert( MinorSubKeyTableType::value_type( subject_major, std::set< uint16 >() ) ).first;
	}

	FATAL_ASSERT( minor_iter->second.find( minor_sub_key ) == minor_iter->second.end() );
	minor_iter->second.insert( minor_sub_key );
}

/**********************************************************************************************************************
	CThreadKeyManager::Remove_Tracked_Thread_Key -- stops tracking a fully-instantiated key

		key -- the unique, fully instantiated key to stop tracking
					
**********************************************************************************************************************/
void CThreadKeyManager::Remove_Tracked_Thread_Key( const SThreadKey &key )
{
	EThreadSubject subject = TS_INVALID;
	uint16 major_sub_key = INVALID_SUB_KEY;
	uint16 minor_sub_key = INVALID_SUB_KEY;

	key.Get_Key_Parts( subject, major_sub_key, minor_sub_key );

	// remove the major key from tracking
	auto major_iter = UsedMajorSubKeys.find( subject );
	FATAL_ASSERT( major_iter != UsedMajorSubKeys.end() );

	major_iter->second.erase( major_sub_key );

	uint64 subject_major = SThreadKey::Build_Key( subject, major_sub_key, 0 );

	// remove the minor key from tracking
	auto minor_iter = UsedMinorSubKeys.find( subject_major );
	FATAL_ASSERT( minor_iter != UsedMinorSubKeys.end() );

	minor_iter->second.erase( minor_sub_key );
}

/**********************************************************************************************************************
	CThreadKeyManager::Find_Next_Major_Sub_Key -- looks up the first unused major sub key for a given thread subject,
		crashes if none available (no recovery)

		subject -- subject to return the first unused major sub key for

		Returns: the first unused major sub key
					
**********************************************************************************************************************/
uint16 CThreadKeyManager::Find_Next_Major_Sub_Key( EThreadSubject subject )
{
	auto major_iter = UsedMajorSubKeys.find( subject );
	if ( major_iter == UsedMajorSubKeys.end() )
	{
		// no major keys for this subject have been used; start at 1 (zero indicates fatal error)
		UsedMajorSubKeys.insert( MajorSubKeyTableType::value_type( subject, std::set< uint16 >() ) );
		return 1;
	}
	
	// loop through all used keys looking for the first "hole" in the integer sequence starting at 1
	uint16 expected_key = 1;
	for ( auto key_iter = major_iter->second.cbegin(); key_iter != major_iter->second.cend(); ++key_iter, ++expected_key )
	{
		if ( *key_iter != expected_key )
		{
			break;
		}
	}

	FATAL_ASSERT( expected_key != 0 );

	return expected_key;
}

/**********************************************************************************************************************
	CThreadKeyManager::Find_Next_Minor_Sub_Key -- looks up the first unused minor sub key for a given thread subject
		and major sub key pair; crashes if none available (no recovery)

		subject -- subject part of the subject-major pair to return the first unused minor sub key for
		major_sub_key -- major part of the subject-major pair to return the first unused minor sub key for

		Returns: the first unused minor sub key
					
**********************************************************************************************************************/
uint16 CThreadKeyManager::Find_Next_Minor_Sub_Key( EThreadSubject subject, uint16 major_sub_key )
{
	uint64 subject_major = SThreadKey::Build_Key( subject, major_sub_key, 0 );
	auto minor_iter = UsedMinorSubKeys.find( subject_major );
	if ( minor_iter == UsedMinorSubKeys.end() )
	{
		// no minor keys for this subject-major pair have been used; start at 1 (zero indicates fatal error)
		UsedMinorSubKeys.insert( MinorSubKeyTableType::value_type( subject, std::set< uint16 >() ) );
		return 1;
	}
	
	// loop through all used keys looking for the first "hole" in the integer sequence starting at 1
	uint16 expected_key = 1;
	for ( auto key_iter = minor_iter->second.cbegin(); key_iter != minor_iter->second.cend(); ++key_iter, ++expected_key )
	{
		if ( *key_iter != expected_key )
		{
			break;
		}
	}

	FATAL_ASSERT( expected_key != 0 );

	return expected_key;
}

