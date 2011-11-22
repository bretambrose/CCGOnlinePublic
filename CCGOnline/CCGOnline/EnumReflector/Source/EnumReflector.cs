using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.IO;

namespace EnumReflector
{
	enum EExecutionMode
	{
		Normal,
		Clean
	}

	class CEnumReflector
	{
		static CEnumReflector()
		{
			Mode = EExecutionMode.Normal;

			ProjectTracker = new CProjectTracker();
			HeaderFileTracker = new CHeaderFileTracker();
			EnumTracker = new CEnumTracker();
		}

		private static void Process_Command_Line_Arguments( string[] arguments )
		{
			foreach ( var argument in arguments )
			{
				string upper_arg = argument.ToUpper();

				if ( upper_arg == "CLEAN" )
				{
					Mode = EExecutionMode.Clean;
				}
				else
				{
					Console.WriteLine( "Unknown command line argument: " + argument );
				}
			}
		}

		private static void Handle_Project_Subdirectory( DirectoryInfo subdirectory )
		{
			foreach ( var file_info in subdirectory.GetFiles( "*.vsproj" ) )
			{
				ProjectTracker.Register_Project( file_info );
			}
		}

		private static void Build_Project_And_File_Sets()
		{
			DirectoryInfo directory_info = new DirectoryInfo( TopLevelDirectory );

			foreach ( var subdirectory_info in directory_info.GetDirectories() )
			{
				Handle_Project_Subdirectory( subdirectory_info );
			}
		}

		private static void Main( string[] args )
		{
			Process_Command_Line_Arguments( args );

			Directory.SetCurrentDirectory( "../.." );

			CEnumXMLDatabase.Load_Config();

			Build_Project_And_File_Sets();
		}

		// Properties
		private static EExecutionMode Mode { get; set; }

		public static CProjectTracker ProjectTracker { get; private set; }
		public static CHeaderFileTracker HeaderFileTracker { get; private set; }
		public static CEnumTracker EnumTracker { get; private set; }
	
		private const string TopLevelDirectory = "";
	}
}
