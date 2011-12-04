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

		private static void Main( string[] args )
		{
			Process_Command_Line_Arguments( args );

			Directory.SetCurrentDirectory( "../.." );

			CEnumXMLDatabase.Load_Config();

			ProjectTracker.Initialize_DB_Projects();
			HeaderFileTracker.Initialize_DB_Header_Files();
			EnumTracker.Initialize_DB_Enums();

			ProjectTracker.Initialize_File_Projects();
			EnumTracker.Initialize_Starting_Enum_States();

			HeaderFileTracker.Process_Dirty_Headers();
			EnumTracker.Process_Final_States();
			ProjectTracker.Write_Enum_Registration_Files();

			CEnumXMLDatabase.Instance.Initialize_From_Trackers( ProjectTracker.SaveRecords, HeaderFileTracker.SaveRecords, EnumTracker.SaveRecords );
			CEnumXMLDatabase.Save_Config();
		}

		// Properties
		private static EExecutionMode Mode { get; set; }

		public static CProjectTracker ProjectTracker { get; private set; }
		public static CHeaderFileTracker HeaderFileTracker { get; private set; }
		public static CEnumTracker EnumTracker { get; private set; }

		public static string TopLevelDirectory { get { return TOP_LEVEL_DIRECTORY; } }
	
		private const string TOP_LEVEL_DIRECTORY = "." + "\\" + "CCGOnline" + "\\";
	}
}
