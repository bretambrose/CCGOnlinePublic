using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.IO;

using Antlr.Runtime;
using Antlr.Runtime.Misc;
using Antlr.Runtime.Tree;

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
			BuildSuffix = null;
			Mode = EExecutionMode.Normal;

			ProjectTracker = new CProjectTracker();
			HeaderFileTracker = new CHeaderFileTracker();
			EnumTracker = new CEnumTracker();
		}

		private static void Process_Command_Line_Arguments( string[] arguments )
		{
			if ( arguments.Length != 3 )
			{
				throw new Exception( "EnumReflector expects three arguments: execution mode (NORMAL/CLEAN), top level directory path, and database suffix (R32/R64/D32/D64)" );
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

			TopLevelDirectory = arguments[ 1 ];

			string upper_arg3 = arguments[ 2 ].ToUpper();
			if ( upper_arg3 == "R32" || upper_arg3 == "R64" || upper_arg3 == "D32" || upper_arg3 == "D64" )
			{
				BuildSuffix = upper_arg3;
			}
			else
			{
				throw new Exception( "Illegal third argument (db suffix): must be 'D32', 'D64', 'R32', or 'R64'" );
			}
		}



		private static void Main( string[] args )
		{
			Process_Command_Line_Arguments( args );

			Directory.SetCurrentDirectory( "../.." );

			if ( Mode != EExecutionMode.Clean )
			{
				CEnumXMLDatabase.Load_Config();
			}

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

		public static string BuildSuffix { get; private set; }

		public static string TopLevelDirectory { get; private set; }
	
	}
}
