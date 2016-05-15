/**********************************************************************************************************************

	LogInterface.cs		
 		A static class used to write out progress, errors, and diagnostics to a text file
 
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
using System.Diagnostics;
using System.Text;

namespace IPCodeGen
{
	public class CLogInterface
	{
		static CLogInterface() {}

		public static void Initialize()
		{
			// Create the log directory if it doesn't exist
			string directory_path = "Run/Tools/Logs";
			if ( !Directory.Exists( directory_path ) )
			{
				Directory.CreateDirectory( directory_path );
			}

			// Clear out any old log files
			string[] file_names  = Directory.GetFiles( directory_path );
			foreach ( string file_name in file_names )
			{
				string local_name = Path.GetFileName( file_name );

				if ( local_name.EndsWith( ".txt" ) && local_name.StartsWith( "IPCodeGenLog_" ) )
				{
					FileInfo log_file = new FileInfo( file_name );
					DateTime last_write_time = log_file.LastWriteTime;

					// skip anything newer than an hour ago just to be cautious
					last_write_time = last_write_time.AddHours( 1 );
					if ( last_write_time >= DateTime.Now )
					{
						continue;
					}

					try
					{
						File.Delete( file_name );
					}
					catch	// locked/read-only files are currently in use; just ignore the exception since we only want to clean up old files
					{
					}
				}
			}

			m_FileName = directory_path + "/IPCodeGenLog_" + Process.GetCurrentProcess().Id.ToString() + ".txt";
			m_FileStream = new FileStream( m_FileName, FileMode.Create, FileAccess.Write, FileShare.Read );
			m_TextWriter = new StreamWriter( m_FileStream );
		}

		public static void Shutdown()
		{
			if ( m_TextWriter != null )
			{
				m_TextWriter.Flush();
				m_TextWriter.Close();
				m_TextWriter = null;
			}

			if ( m_FileStream != null )
			{
				m_FileStream.Close();
				m_FileStream = null;
			}
		}

		public static void Write( string message )
		{
			lock(m_TextWriter)
			{
				m_TextWriter.Write( message );
			}
		}

		public static void Write_Line( string message )
		{
			lock(m_TextWriter)
			{
				m_TextWriter.WriteLine( message );
			}
		}

		public static void Write_Exception( Exception e )
		{
			StringBuilder exception_text = new StringBuilder();

			exception_text.AppendLine();
			exception_text.AppendLine( "******EXCEPTION*******" );
			exception_text.AppendLine( "Message:" );
			exception_text.AppendLine( e.Message );
			exception_text.AppendLine();
			exception_text.AppendLine( "StackTrace:" );
			exception_text.AppendLine( e.StackTrace );
			exception_text.AppendLine();

			Write_Line(exception_text.ToString());
		}

		public static string Get_Log_File_Name()
		{
			return m_FileName;
		}

		private static string m_FileName = null;
		private static FileStream m_FileStream = null;
		private static StreamWriter m_TextWriter = null;
	}
}

