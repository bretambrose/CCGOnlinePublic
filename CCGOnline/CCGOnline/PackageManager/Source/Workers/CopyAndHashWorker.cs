/**********************************************************************************************************************

	[Placeholder for eventual source license]

	CopyAndHashWorker.cs
		A background worker that copies a directory or set of files within a directory, hashing the files as it goes.

	(c) Copyright 2011, Bret Ambrose.  All rights reserved.

**********************************************************************************************************************/

using System;
using System.ComponentModel;
using System.IO;
using System.Security.Cryptography;
using System.Collections.Generic;

namespace PackageManager
{
	public class CCopyAndHashWorker : CWorker
	{
		public CCopyAndHashWorker( string output_source, string output_destination )
		{
			CumulativeHash = new CHash();
			OutputSource = output_source;
			OutputDestination = output_destination;
		}

		protected override void OnDoWork( DoWorkEventArgs event_args )
		{
			if ( Directory.Exists( OutputSource ) )
			{
				DirectoryInfo directory_info = new DirectoryInfo( OutputSource );

				directory_info.GetFiles().Apply( f => m_PendingFiles.Enqueue( f ) );
				directory_info.GetDirectories().Apply( d => m_PendingDirectories.Enqueue( d ) );

				string []directory_split = OutputSource.Split( '/' );
				BaseDirectory = directory_split[ directory_split.Length - 1 ];
			}
			else
			{
				string []directory_split = Path.GetDirectoryName( OutputSource ).Split( '/' );
				BaseDirectory = directory_split[ directory_split.Length - 1 ];

				DirectoryInfo directory_info = new DirectoryInfo( BaseDirectory );
				string file_pattern = Path.GetFileName( OutputSource );

				directory_info.GetFiles( file_pattern ).Apply( f => m_PendingFiles.Enqueue( f ) );
			}

			while ( m_PendingFiles.Count > 0 || m_PendingDirectories.Count > 0 )
			{
				Process_Pending_Files_And_Directories();
			}
		}

		private void Process_Pending_Files_And_Directories()
		{
			while ( m_PendingFiles.Count > 0 || m_PendingDirectories.Count > 0 )
			{
				if ( m_PendingFiles.Count > 0 )
				{
					while ( m_PendingFiles.Count > 0 )
					{
						FileInfo file_info = m_PendingFiles.Dequeue();
						Copy_And_Hash_File( file_info );
					}
				}
				else if ( m_PendingDirectories.Count > 0 )
				{
					DirectoryInfo directory_info = m_PendingDirectories.Dequeue();

					directory_info.GetFiles().Apply( f => m_PendingFiles.Enqueue( f ) );
					directory_info.GetDirectories().Apply( d => m_PendingDirectories.Enqueue( d ) );
				}
			}
		}

		private void Copy_And_Hash_File( FileInfo file_info )
		{
			using ( FileStream ifs = File.OpenRead( file_info.FullName ) )
			{
				ifs.Position = 0;
				CumulativeHash.Fold( m_Hasher.ComputeHash( ifs ) );

				int pos = file_info.FullName.LastIndexOf( BaseDirectory );
				string substring = file_info.FullName.Substring( pos + BaseDirectory.Length );
				string output_file_name = OutputDestination + substring;

				string directory_name = Path.GetDirectoryName( output_file_name );
				
				if ( directory_name.Length > 0 ) 
				{
					Directory.CreateDirectory( directory_name );
				}

				using ( FileStream ofs = File.OpenWrite( output_file_name ) )
				{
					ofs.Position = 0;
					ifs.Position = 0;

					ifs.CopyTo( ofs );

					ofs.Close();
				}

				ifs.Close();
			}
		}

		public CHash CumulativeHash { get; private set; }

		private Queue< FileInfo > m_PendingFiles = new Queue< FileInfo >();
		private Queue< DirectoryInfo > m_PendingDirectories = new Queue< DirectoryInfo >();

		private MD5 m_Hasher = MD5.Create();

		private string OutputSource = null;
		private string OutputDestination = null;
		private string BaseDirectory = null;

	}
}