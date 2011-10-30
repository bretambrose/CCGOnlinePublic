/**********************************************************************************************************************

	Hash.cs
		Wrapper class for a file or directory hash.

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

namespace PackageManager
{
	public class CHash : IEquatable< CHash >
	{
		public CHash()
		{
		}

		public CHash( byte []hash_bytes )
		{
			m_HashBytes = hash_bytes;
		}

		public void Fold( byte []hash_bytes )
		{
			if ( m_HashBytes == null )
			{
				m_HashBytes = hash_bytes;
				return;
			}

			if ( hash_bytes == null )
			{
				throw new Exception( "Attempt to fold a null hash into an existing hash" );
			}

			if ( hash_bytes.Length != m_HashBytes.Length )
			{
				throw new Exception( "Attempt to fold hashes with unequal length" );
			}

			for ( int i = 0; i < m_HashBytes.Length; i++ )
			{
				m_HashBytes[ i ] ^= hash_bytes[ i ];
			}
		}

		public void Fold( CHash hash )
		{
			Fold( hash.Get_Raw_Bytes() );
		}

		public override int GetHashCode()
		{
			if ( m_HashBytes == null )
			{
				return 0;
			}

			return m_HashBytes.GetHashCode();
		}

		public override bool Equals( Object obj )
		{
			if ( obj is CHash )
			{
				return Equals( obj as CHash );
			}

			return false;
		}

		public bool Equals( CHash hash )
		{
			if ( m_HashBytes == null )
			{
				return hash.m_HashBytes == null;
			}

			if ( hash.m_HashBytes == null )
			{
				return false;
			}

			if ( m_HashBytes.Length != hash.m_HashBytes.Length )
			{
				return false;
			}

			for ( int i = 0; i < m_HashBytes.Length; ++i )
			{
				if ( m_HashBytes[ i ] != hash.m_HashBytes[ i ] )
				{
					return false;
				}
			}

			return true;
		}

		public byte []Get_Raw_Bytes()
		{
			return m_HashBytes;
		}

		public bool Is_Valid()
		{
			return m_HashBytes != null;
		}

		private byte []m_HashBytes = null;
	}
}
