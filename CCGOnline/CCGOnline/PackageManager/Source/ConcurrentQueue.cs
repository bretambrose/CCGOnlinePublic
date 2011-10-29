/**********************************************************************************************************************

	[Placeholder for eventual source license]

	ConcurrentQueue.cs
		Simple lock-based concurrent queue.  Used by the hashing system which chains multiple background workers to
		split up the worker for large directory trees.  This kind of hashing is not really necessary, but now that
		it's done, there's no point in removing it.

	(c) Copyright 2011, Bret Ambrose.  All rights reserved.

**********************************************************************************************************************/

using System;
using System.Collections.Generic;

namespace PackageManager
{
	public class CLockedQueue< T >
	{
		// Construction
		public CLockedQueue()
		{
		}
		
		// Methods
		// Public interface
		public void Add( T item )
		{
			lock( m_Lock )
			{
				m_Queue.Enqueue( item );
			}
		}
		
		public void Take_All( ICollection< T > dest_collection )
		{
			lock( m_Lock )
			{
				m_Queue.ShallowCopy( dest_collection );
				m_Queue.Clear();
			}
		}
		
		// Fields
		private Queue< T > m_Queue = new Queue< T >();
		private object m_Lock = new object();
		
	}
}

