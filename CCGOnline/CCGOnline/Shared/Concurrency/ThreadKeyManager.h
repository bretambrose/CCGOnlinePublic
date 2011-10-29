/**********************************************************************************************************************

	[Placeholder for eventual source license]

	ThreadKeyManager.h
		A component definining a class that allocates and tracks thread keys

	(c) Copyright 2011, Bret Ambrose.  All rights reserved.

**********************************************************************************************************************/

#ifndef THREAD_KEY_MANAGER_H
#define THREAD_KEY_MANAGER_H

struct SThreadKey;

enum EThreadSubject;

// a class that allocates and tracks thread keys
class CThreadKeyManager
{
	public:

		CThreadKeyManager( void );

		SThreadKey Fill_In_Thread_Key( const SThreadKey &base_key );

		void Add_Tracked_Thread_Key( const SThreadKey &key );
		void Remove_Tracked_Thread_Key( const SThreadKey &key );

	private:

		uint16 Find_Next_Major_Sub_Key( EThreadSubject subject );
		uint16 Find_Next_Minor_Sub_Key( EThreadSubject subject, uint16 major_sub_key );

		typedef stdext::hash_map< EThreadSubject, std::set< uint16 > > MajorSubKeyTableType;

		// The key is a bitwise combination of both subject and major key
		typedef stdext::hash_map< uint64, std::set< uint16 > > MinorSubKeyTableType;

		MajorSubKeyTableType UsedMajorSubKeys;
		MinorSubKeyTableType UsedMinorSubKeys;

};

#endif // THREAD_KEY_MANAGER_H
