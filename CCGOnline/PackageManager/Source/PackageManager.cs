/**********************************************************************************************************************

	PackageManager.cs
		Entry component for the package manager.  Manages all input and output states, cleans appropriate directories.

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
using System.Text;
using System.Linq;
using System.Threading;
using System.IO;

using System.Net;

namespace PackageManager
{
	enum EExecutionMode
	{
		Normal,
		Clean
	}

	class CPackageManager
	{
		static CPackageManager()
		{
			Mode = EExecutionMode.Normal;
			GenerateConfigFile = false;
		}

		public static uint Get_Input_Packages_In_State_Count( EInputPackageState state )
		{
			return (uint) m_PackageInputs.Values.Where( n => n.State == state ).Count();
		}

		public static uint Get_Output_Packages_In_State_Count( EOutputPackageState state )
		{
			return (uint) m_PackageOutputs.Values.Where( n => n.State == state ).Count();
		}

		public static CPackageInputState Get_Package_Input( EPackageInputID id )
		{
			CPackageInputState input = null;
			m_PackageInputs.TryGetValue( id, out input );

			return input;
		}

		public static CPackageOutputState Get_Package_Output( EPackageOutputID id )
		{
			CPackageOutputState output = null;
			m_PackageOutputs.TryGetValue( id, out output );

			return output;
		}

		public static EOperatingSystem Get_OS()
		{
			return EOperatingSystem.Windows;
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
				else if ( upper_arg == "GENCONFIG" )
				{
					GenerateConfigFile = true;
				}
				else
				{
					Console.WriteLine( "Unknown command line argument: " + argument );
				}
			}
		}

		private static void Wait_For_Error_Acknowledgement()
		{
			Console.WriteLine( "Error running package manager.  Stopping execution." );
			Console.WriteLine( "Hit any key to continue." );
			Console.ReadKey( false );
			return;
		}

		private static void Initialize_Directories()
		{
			Directory.SetCurrentDirectory( "../.." );

			if ( !Directory.Exists( DownloadDirectory ) ) 
			{
				// Try to create the directory.
				Directory.CreateDirectory( DownloadDirectory );
			}

			CFileUtils.Clean_Directory( DownloadDirectory );

			if ( !Directory.Exists( UnpackDirectory ) )
			{
				Directory.CreateDirectory( UnpackDirectory );
			}

			CFileUtils.Clean_Directory( UnpackDirectory );		
		}

		private static bool Validate_Config_And_Initialize()
		{
			foreach ( var input in CConfigSettings.Instance.Inputs )
			{
				string upper_name = input.Name.ToUpper();
				if ( m_PackageInputNamesTable.ContainsKey( upper_name ) )
				{
					throw new Exception( "Two input packages have the same name: " + upper_name );
				}

				CPackageInputState input_state = new CPackageInputState( input );
				m_PackageInputNamesTable.Add( upper_name, input_state.ID );
				m_PackageInputs.Add( input_state.ID, input_state );
			}

			foreach ( var output in CConfigSettings.Instance.Outputs )
			{
				EPackageInputID input_id = EPackageInputID.Invalid;
				string source_name = output.PackageName.ToUpper();
				if ( !m_PackageInputNamesTable.TryGetValue( source_name, out input_id ) )
				{
					throw new Exception( "Output entry has invalid package source: " + output.PackageName );					
				}

				string tag = output.Tag.ToUpper();
				if ( m_PackageOutputNamesTable.ContainsKey( tag ) )
				{
					throw new Exception( "Two output entries have the same tag: " + output.Tag );				
				}

				CPackageOutputState output_state = new CPackageOutputState( output, input_id );
				m_PackageOutputNamesTable.Add( tag, output_state.ID );
				m_PackageOutputs.Add( output_state.ID, output_state );
			}

			return true;
		}

		private static void Initialize_Output_States()
		{
			// figure out which outputs are dirty
			foreach ( var output_manifest_entry in COutputManifest.Instance.Outputs )
			{
				EPackageOutputID id = EPackageOutputID.Invalid;
				if ( !m_PackageOutputNamesTable.TryGetValue( output_manifest_entry.OutputTag.ToUpper(), out id ) )
				{
					continue;
				}

				CPackageOutputState package_output = Get_Package_Output( id );
				if ( package_output == null )
				{
					continue;
				}

				if ( !package_output.Config.Hash.Equals( output_manifest_entry.Hash ) || !output_manifest_entry.Hash.Is_Valid() )
				{
					continue;
				}

				package_output.State = EOutputPackageState.Finished;
			}

			// if a dirty and a non-dirty output share destination directories, we need to dirty the non-dirty as a simplification measure
			foreach ( var output_outer in m_PackageOutputs.Values )
			{
				if ( output_outer.State == EOutputPackageState.Finished )
				{
					continue;
				}

				foreach ( var output_inner in m_PackageOutputs.Values )
				{
					if ( output_outer.Config.Destination == output_inner.Config.Destination )
					{
						output_inner.State = output_outer.State;
					}
				}
			}

			// create and clear all dirty destination directories
			foreach ( var output_entry in m_PackageOutputs.Values )
			{
				if ( output_entry.State != EOutputPackageState.Finished )
				{
					string dest_directory = output_entry.Config.Destination;
					Directory.CreateDirectory( dest_directory );
					CFileUtils.Clean_Directory( dest_directory );
				}
			}
		}

		private static void Initialize_Input_States()
		{
			foreach ( var input_state in m_PackageInputs.Values )
			{
				input_state.State = EInputPackageState.Finished;
			}

			foreach ( var output_state in m_PackageOutputs.Values )
			{
				if ( output_state.State == EOutputPackageState.Finished )
				{
					continue;
				}

				Get_Package_Input( output_state.InputID ).State = EInputPackageState.Start;
			}
		}

		private static void Initialize()
		{
			Initialize_Directories();

			CConfigSettings.Load_Config();

			Validate_Config_And_Initialize();

			if ( Mode == EExecutionMode.Clean || GenerateConfigFile )
			{
				COutputManifest.Delete();
			}
			else
			{
				COutputManifest.Load();
			}
			
			if ( GenerateConfigFile )
			{
				CConfigSettings.Instance.Discard_Hashes();
			}

			Initialize_Output_States();
			Initialize_Input_States();
		}

		private static bool Service_Packages()
		{
			foreach ( var input_package in m_PackageInputs.Values )
			{
				input_package.Service();
			}

			foreach ( var package_output in m_PackageOutputs.Values )
			{
				package_output.Service();
			}

			return Get_Output_Packages_In_State_Count( EOutputPackageState.Finished ) == m_PackageOutputs.Count;
		}

		private static void Main( string[] args )
		{
			Process_Command_Line_Arguments( args );
					
			try
			{
				Initialize();

				bool done = false;
				while ( !done )
				{
					done = Service_Packages();
					Thread.Sleep( 0 );
				}

				COutputManifest.Save();
				if ( GenerateConfigFile )
				{
					CConfigSettings.Save_Config();
				}
			}
			catch ( Exception e )
			{
				Console.WriteLine( e.Message );
				Wait_For_Error_Acknowledgement();
			}
		}

		// Properties
		public static string DownloadDirectory { get { return @"Run/Tools/Temp/PackageManager/Downloads"; } }
		public static string UnpackDirectory { get { return @"Run/Tools/Temp/PackageManager/Unpack"; } }

		private static EExecutionMode Mode { get; set; }
		private static bool GenerateConfigFile { get; set; }

		// Fields
		
		private static Dictionary< EPackageInputID, CPackageInputState > m_PackageInputs = new Dictionary< EPackageInputID, CPackageInputState >();
		private static Dictionary< EPackageOutputID, CPackageOutputState > m_PackageOutputs = new Dictionary< EPackageOutputID, CPackageOutputState >();

		private static Dictionary< string, EPackageInputID > m_PackageInputNamesTable = new Dictionary< string, EPackageInputID >();
		private static Dictionary< string, EPackageOutputID > m_PackageOutputNamesTable = new Dictionary< string, EPackageOutputID >();

	}
}
