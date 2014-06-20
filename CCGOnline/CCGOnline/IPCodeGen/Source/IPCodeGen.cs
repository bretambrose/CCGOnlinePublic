/**********************************************************************************************************************

	IPCodeGen.cs
		Main definition for the IPCodeGen tool.  This tool ...

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
using System.Linq;
using System.Text;
using System.IO;
using System.Threading;


namespace IPCodeGen
{

	enum EExecutionMode
	{
		Normal,
		Clean
	}

	class CIPCodeGen
	{
		// Construction
		static CIPCodeGen()
		{
			Mode = EExecutionMode.Normal;

			OutputDirectory = "Run/Tools/Data/XML/IPCodeGen";
		}

		// Methods
		// Private interface
		private static void Process_Command_Line_Arguments( string[] arguments )
		{
			if ( arguments.Length != 2 )
			{
				throw new Exception( "IpCodeGen expects two arguments: execution mode (NORMAL/CLEAN), and top level directory path" );
			}

			string upper_arg1 = arguments[ 0 ].ToUpper();
			if ( upper_arg1 == "CLEAN" )
			{
				Mode = EExecutionMode.Clean;
			}
			else if ( upper_arg1 == "NORMAL" )
			{
				Mode = EExecutionMode.Normal;
			}
			else
			{
				throw new Exception( "Illegal first argument (execution mode): must be either 'Normal' or 'Clean'" );
			}

			TopLevelDirectory = arguments[ 1 ] + Path.DirectorySeparatorChar;
		}

		private static void Clean_Output_Files()
		{
			string[] file_names  = Directory.GetFiles( OutputDirectory );
			foreach ( string file_name in file_names )
			{
				try
				{
					File.Delete( file_name );
				}
				catch	
				{
				}
			}
		}

		private static int Main( string[] args )
		{
			Mutex global_lock = new Mutex( false, "IPCodeGen" );

			// Prevent multiple copies of IPCodeGen from interfering with one another if a batch build is in progress
			// Runs after the first should detect no changes and not do any writes
			if ( !global_lock.WaitOne() )
			{
				return 1;
			}

			int error_level = 0;
			try
			{
				DateTime start_time = DateTime.Now;
				Directory.SetCurrentDirectory( "../.." );

				CLogInterface.Initialize();

				CLogInterface.Write_Line( "IPCodeGen" );
				CLogInterface.Write_Line( "Starting processing" );

				Process_Command_Line_Arguments( args );

				if ( Mode == EExecutionMode.Clean )
				{
					Clean_Output_Files();
				}

				CCodeGenTaskTracker task_tracker = new CCodeGenTaskTracker();
				task_tracker.Run();

				DateTime end_time = DateTime.Now;
				TimeSpan run_time = end_time.Subtract( start_time );
				CLogInterface.Write_Line( "Total time taken: " + run_time.ToString() );
			}
			catch ( Exception e )
			{
				CLogInterface.Write_Exception( e );

				Console.WriteLine( "There was an error running IPCodeGen." );
				Console.WriteLine( "See " + CLogInterface.Get_Log_File_Name() + " for details.\n" );
				Console.WriteLine( "Hit any key to continue" );
				Console.ReadKey();
				error_level = 1;
			}
			finally
			{
				CLogInterface.Shutdown();
				global_lock.ReleaseMutex();
			}

			return error_level;
		}

		// Properties
		private static EExecutionMode Mode { get; set; }

		public static string TopLevelDirectory { get; private set; }
		public static string OutputDirectory { get; private set; }
	
	}
}
