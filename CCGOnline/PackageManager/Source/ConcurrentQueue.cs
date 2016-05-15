/**********************************************************************************************************************

	ConcurrentQueue.cs
		Simple lock-based concurrent queue.  Used by the hashing system which chains multiple background workers to
		split up the worker for large directory trees.  This kind of hashing is not really necessary, but now that
		it's done, there's no point in removing it.

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

