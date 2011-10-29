/**********************************************************************************************************************

	[Placeholder for eventual source license]

	DecompressWorker.cs
		A background worker that decompresses a package.

	(c) Copyright 2011, Bret Ambrose.  All rights reserved.

**********************************************************************************************************************/

using System;
using System.ComponentModel;
using System.IO;

using ICSharpCode.SharpZipLib.Zip;

namespace PackageManager
{
	public class CDecompressWorker : CWorker
	{
		public CDecompressWorker( string target_filename, string directory_prefix )
		{
			TargetFilename = target_filename;
			DirectoryPrefix = directory_prefix + "\\";
		}

		// Adapted from SharpZipLib's UnZipFile example
		protected override void OnDoWork( DoWorkEventArgs event_args )
		{
			using ( FileStream fs = File.OpenRead( TargetFilename ) )
			using ( ZipInputStream zs = new ZipInputStream( fs ) )
			{
				byte[] output_buffer = new byte[ OUTPUT_BUFFER_SIZE ];

				ZipEntry zip_entry;
				while ( ( zip_entry = zs.GetNextEntry() ) != null ) 
				{
					string directory_name = DirectoryPrefix + Path.GetDirectoryName( zip_entry.Name );
					string filename = Path.GetFileName( zip_entry.Name );
				
					if ( directory_name.Length > 0 ) 
					{
						Directory.CreateDirectory( directory_name );
					}

					if ( filename == String.Empty ) 
					{
						continue;
					}

					using ( FileStream os = File.Create( DirectoryPrefix + zip_entry.Name ) ) 
					{
						int size = zs.Read( output_buffer, 0, output_buffer.Length );
						while ( size > 0 )
						{
							os.Write( output_buffer, 0, size );
							size = zs.Read( output_buffer, 0, output_buffer.Length );
						}

						os.Close();
					}
				}

				zs.Close();
				fs.Close();
			}
		}

		private string TargetFilename = null;
		private string DirectoryPrefix = null;

		private const int OUTPUT_BUFFER_SIZE = 8192;
	}
}