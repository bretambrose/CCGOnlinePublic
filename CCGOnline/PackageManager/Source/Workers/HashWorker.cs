/**********************************************************************************************************************

	HashWorker.cs
		A set of background workers for hashing a file or directory.  There are two different implementations here
		for performance comparison purposes.  CHashWorker2 is a singlethreaded implementation that iterates every
		file and hashes it.  CHashWorker is a more parallel implementation that farms out sets of files to
		a controllable, bounded set of background workers for hashing.

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
using System.Net;
using System.Text;
using System.IO;
using System.ComponentModel;
using System.Collections.Generic;
using System.Threading;
using System.Security.Cryptography;

/*

I did some simple performance testing comparing CHashWorker(8 subworkers) vs. CHashWorker2 on the entire Boost_1_47_0 directory.

							Cold OS Cache			Hot OS Cache

	CHashWorker(8)		186.5s					1.3s

	CHashWorker2		238s						4.4s

Hot vs. Cold is determined by first OS access.  Once accessed the OS appears to cache the file in memory making subsequent hash
attempts much, much faster.  Unfortunately for actual users of the package manager, they will never see this performance boost because
they will only run the process once.

The testing procedure looked like (note that switching hot and cold requires a swap in the order of 1 vs 2 in the function:
 
 
		private static bool Test_Hashing()
		{
			string hash_target = "C:\\GitProjects\\ProjectsBase\\CCGOnline\\CCGOnline\\External\\boost";

			long start2 = DateTime.Now.Ticks;
			var worker2 = new CHashWorker2( hash_target );
			worker2.RunWorkerAsync();
			
			while ( worker2.Get_Completion_Status() == EWorkStatus.InProgress )
			{
				Thread.Sleep( 1 );
			}
			long end2 = DateTime.Now.Ticks;

			double time2 = ( (double) end2 - (double) start2 ) / 10000000.0;

			Console.WriteLine( "Hash time2: " + time2 );

			long start1 = DateTime.Now.Ticks;
			var worker = new CHashWorker( hash_target, 8 );
			worker.RunWorkerAsync();
			
			while ( worker.Get_Completion_Status() == EWorkStatus.InProgress )
			{
				Thread.Sleep( 1 );
			}
			long end1 = DateTime.Now.Ticks;
			double time1 = ( (double) end1 - (double) start1 ) / 10000000.0;

			Console.WriteLine( "Hash time1: " + time1 );

			return true;
		}
 
*/

namespace PackageManager
{
	public enum EHashWorkID
	{
		Invalid
	}

	public class CHashResponse
	{
		public CHashResponse( EHashWorkID id, CHash hash )
		{
			ID = id;
			Hash = hash;
		}

		public EHashWorkID ID { get; private set; }
		public CHash Hash { get; private set; }
	}

	public delegate void DHashResponse( CHashResponse response );

	public class CHashSubWorker : CWorker
	{
		// Construction
		public CHashSubWorker( EHashWorkID id, DHashResponse response_delegate )
		{
			ID = id;
			m_ResponseDelegate = response_delegate;
		}

		// Methods
		// Public interface
		public void Add_File( FileInfo file ) 
		{ 
			m_Files.Add( file ); 
		}

		// Protected interface
		protected override void OnDoWork( DoWorkEventArgs event_args )
		{
			try
			{
				MD5 hasher = MD5.Create();

				foreach ( var file in m_Files )
				{
					using ( FileStream stream = File.OpenRead( file.FullName ) )
					{
						stream.Position = 0;
						m_CumulativeHash.Fold( hasher.ComputeHash( stream ) );
						stream.Close();
					}
				}

				m_ResponseDelegate( new CHashResponse( ID, m_CumulativeHash ) );
			}
			catch ( Exception e )
			{
				m_ResponseDelegate( new CHashResponse( ID, null ) );
				throw e;
			}
		}

		// Properties
		public EHashWorkID ID { get; private set; }

		// Fields
		private List< FileInfo > m_Files = new List< FileInfo >();
		private CHash m_CumulativeHash = new CHash();
		private DHashResponse m_ResponseDelegate = null;
	}

	public class CHashWorker : CWorker
	{
		// Construction
		public CHashWorker( string hash_path, int max_sub_workers )
		{
			m_HashPath = hash_path;
			m_MaxSubWorkers = max_sub_workers;
		}

		// Methods
		// Public interface
		public void Add_Response( CHashResponse response )
		{
			m_Responses.Add( response );
		}

		// Protected interface
		protected override void OnDoWork( DoWorkEventArgs event_args )
		{
			Console.WriteLine( "Hashing path: " + m_HashPath );

			if ( Directory.Exists( m_HashPath ) )
			{
				m_PendingDirectories.Enqueue( new DirectoryInfo( m_HashPath ) );
			}
			else
			{
				m_PendingFiles.Enqueue( new FileInfo( m_HashPath ) );
			}

			try
			{
				while ( m_PendingFiles.Count > 0 || m_PendingDirectories.Count > 0 || m_PendingResponses.Count > 0 )
				{
					Process_Pending_Files_And_Directories();
					Process_Pending_Responses();
					Thread.Sleep( 0 );
				}
			}
			catch ( Exception e )
			{
				Console.WriteLine( "Failed to hash path: " + m_HashPath );
				throw e;
			}
		}

		private void Process_Pending_Files_And_Directories()
		{
			if ( ( m_PendingFiles.Count == 0 && m_PendingDirectories.Count == 0 ) || m_PendingResponses.Count >= m_MaxSubWorkers )
			{
				return;
			}

			CHashSubWorker hash_file_worker = new CHashSubWorker( ++m_CurrentID, n => Add_Response( n ) );
			ulong file_size_total = 0;

			while ( ( m_PendingFiles.Count > 0 || m_PendingDirectories.Count > 0 ) && file_size_total < HASH_SIZE_CUTOFF )
			{
				if ( m_PendingFiles.Count > 0 )
				{
					while ( m_PendingFiles.Count > 0 && file_size_total < HASH_SIZE_CUTOFF )
					{
						FileInfo file_info = m_PendingFiles.Dequeue();
						hash_file_worker.Add_File( file_info );
						file_size_total += (ulong) file_info.Length;
					}
				}
				else if ( m_PendingDirectories.Count > 0 )
				{
					DirectoryInfo directory_info = m_PendingDirectories.Dequeue();

					directory_info.GetFiles().Apply( f => m_PendingFiles.Enqueue( f ) );
					directory_info.GetDirectories().Apply( d => m_PendingDirectories.Enqueue( d ) );
				}
			}

			m_PendingResponses.Add( m_CurrentID );
			hash_file_worker.RunWorkerAsync();
		}

		private void Process_Pending_Responses()
		{
			m_ResponseHolder.Clear();
			m_Responses.Take_All( m_ResponseHolder );

			foreach ( var response in m_ResponseHolder )
			{
				m_PendingResponses.Remove( response.ID );
				m_CumulativeHash.Fold( response.Hash );
			}

			m_ResponseHolder.Clear();
		}

		// Properties
		public CHash Hash { get { return m_CumulativeHash; } }

		// Fields
		private string m_HashPath = null;
		private EHashWorkID m_CurrentID = EHashWorkID.Invalid;

		private CLockedQueue< CHashResponse > m_Responses = new CLockedQueue< CHashResponse >();
		private HashSet< EHashWorkID > m_PendingResponses = new HashSet< EHashWorkID >();
		private List< CHashResponse > m_ResponseHolder = new List< CHashResponse >();


		private Queue< FileInfo > m_PendingFiles = new Queue< FileInfo >();
		private Queue< DirectoryInfo > m_PendingDirectories = new Queue< DirectoryInfo >();

		private CHash m_CumulativeHash = new CHash();

		private int m_MaxSubWorkers;

		private const ulong HASH_SIZE_CUTOFF = 2000000;
	}

	public class CHashWorker2 : CWorker
	{
		// Construction
		public CHashWorker2( string hash_path )
		{
			m_HashPath = hash_path;
		}

		// Methods
		// Protected interface
		protected override void OnDoWork( DoWorkEventArgs event_args )
		{
			if ( Directory.Exists( m_HashPath ) )
			{
				m_PendingDirectories.Enqueue( new DirectoryInfo( m_HashPath ) );
			}
			else
			{
				m_PendingFiles.Enqueue( new FileInfo( m_HashPath ) );
			}

			try
			{
				Process_Pending_Files_And_Directories();
			}
			catch ( Exception e )
			{
				Console.WriteLine( "Failed to hash path: " + m_HashPath );
				throw e;
			}
		}

		private void Process_Pending_Files_And_Directories()
		{
			MD5 hasher = MD5.Create();

			while ( m_PendingFiles.Count > 0 || m_PendingDirectories.Count > 0 )
			{
				while ( m_PendingFiles.Count > 0 )
				{
					FileInfo file_info = m_PendingFiles.Dequeue();

					using ( FileStream stream = File.OpenRead( file_info.FullName ) )
					{
						stream.Position = 0;
						m_CumulativeHash.Fold( hasher.ComputeHash( stream ) );
						stream.Close();
					}
				}

				while ( m_PendingDirectories.Count > 0 )
				{
					DirectoryInfo directory_info = m_PendingDirectories.Dequeue();

					directory_info.GetFiles().Apply( f => m_PendingFiles.Enqueue( f ) );
					directory_info.GetDirectories().Apply( d => m_PendingDirectories.Enqueue( d ) );
				}
			}
		}

		// Properties

		// Fields
		private string m_HashPath = null;

		private Queue< FileInfo > m_PendingFiles = new Queue< FileInfo >();
		private Queue< DirectoryInfo > m_PendingDirectories = new Queue< DirectoryInfo >();

		private CHash m_CumulativeHash = new CHash();
	}
}