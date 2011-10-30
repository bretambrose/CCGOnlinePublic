/**********************************************************************************************************************

	FileUtils.cs
		Miscellaneous file utility functions.  .NET's directory deletion functionality leaves a lot to be desired.  In
		particular, if windows explorer or anti-virus has an open handle to a file within the directory structure,
		an undocumented exception can sometimes occur.  If you keep trying, with a healthy pause inbetween tries, the
		delete will eventually go through.

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
using System.IO;
using System.Threading;
using System.Collections.Generic;
using System.Linq;

namespace PackageManager
{
	public static class CFileUtils
	{
		// Methods
		// Public interface

		// .NETs built in Delete for directories is unreliable, even with the force flag set
		// to true.  See http://stackoverflow.com/questions/329355/cannot-delete-directory-with-directory-deletepath-true
		public static void Clean_Directory( string directory )
		{
			for ( int i = 0; i < 20; i++ )
			{
				try
				{
					Clean_Directory_Aux( directory );
					break;
				}
				catch ( Exception )
				{
					Console.WriteLine( "Trouble deleting directory " + directory );
					Console.WriteLine( "Windows is dumb.  Trying again shortly. " );
					Thread.Sleep( 1000 );
				}
			}
		}

		// Private interface
		private static void Hacky_Delete_Directory( DirectoryInfo dir_info )
		{
			try
			{
				File.SetAttributes( dir_info.FullName, FileAttributes.Normal ); 
				dir_info.Delete( false );
			}
			catch ( IOException )
			{
				Thread.Sleep( 0 );
				dir_info.Delete( false );
			}
		}

		private static void Hacky_Delete_File( FileInfo file_info )
		{
			File.SetAttributes( file_info.FullName, FileAttributes.Normal ); 
			file_info.Delete();
		}

		private static void Clean_Directory_Aux( string directory )
		{
			DirectoryInfo directory_info = new DirectoryInfo( directory );

			Queue< DirectoryInfo > unprocessed_directories = new Queue< DirectoryInfo >();
			Stack< DirectoryInfo > pending_deletes = new Stack< DirectoryInfo >();

			directory_info.GetFiles().ToList().Apply( f => f.Delete() );
			directory_info.GetDirectories().Apply( f => unprocessed_directories.Enqueue( f ) );

			while ( unprocessed_directories.Count > 0 )
			{
				DirectoryInfo dir = unprocessed_directories.Dequeue();
				pending_deletes.Push( dir );

				dir.GetFiles().ToList().Apply( f => Hacky_Delete_File( f ) );
				dir.GetDirectories().Apply( f => unprocessed_directories.Enqueue( f ) );
			}

			Thread.Sleep( 0 );
			pending_deletes.Apply( d => Hacky_Delete_Directory( d ) );
		}


	}
}
