/**********************************************************************************************************************

	ProjectTracker.cs		
 		A pair of classes for tracking the C++ projects that get analyzed by the tool.
 
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
using System.Collections.Generic;
using System.Xml.Linq;
using System.Linq;
using System.Text;

namespace EnumReflector
{
	public enum EProjectID
	{
		Invalid = 0
	}

	public enum EProjectCreationState
	{
		Invalid = 0,

		New,
		Deleted,
		Unchanged
	}

	public enum EProjectState
	{
		Invalid = 0,

		Unknown,
		Dirty,
		Unchanged
	}

	public class CProject
	{
		// Construction
		public CProject( EProjectID id, CProjectRecord old_project_record )
		{
			ID = id;
			OldProjectRecord = old_project_record;
			NewProjectRecord = null;
			CreationState = EProjectCreationState.Deleted;
			State = EProjectState.Invalid;
		} 

		public CProject( EProjectID id, FileInfo project_file )
		{
			ID = id;
			OldProjectRecord = null;
			NewProjectRecord = new CProjectRecord( Path.GetFileNameWithoutExtension( project_file.FullName ) );
			CreationState = EProjectCreationState.New;
			State = EProjectState.Dirty;

			Parse_Project_File( project_file );
		} 

		// Public interface
		public void Initialize_Existing( FileInfo project_file )
		{
			NewProjectRecord = new CProjectRecord( Path.GetFileNameWithoutExtension( project_file.FullName ) );
			CreationState = EProjectCreationState.Unchanged;
			State = EProjectState.Unknown;

			Parse_Project_File( project_file );
		}

		public void Write_Enum_Registration_Files()
		{
			CLogInterface.Write_Line( "Writing enum registration files for project: " + Name );

			string generated_code_directory = Build_Registration_Directory_Path();
			if ( !Directory.Exists( generated_code_directory ) )
			{
				Directory.CreateDirectory( generated_code_directory );
			}

			string header_file_name = Build_Registration_Header_File_Name();
			if ( !File.Exists( header_file_name ) )
			{
				StringBuilder header_file_text = Build_Header_Text( Path.GetFileName( header_file_name ) );
				File.WriteAllText( header_file_name, header_file_text.ToString() );
			}

			string cpp_file_name = Build_Registration_CPP_File_Name();
			StringBuilder cpp_file_text = Build_CPP_Text( Path.GetFileName( cpp_file_name ) );
			File.WriteAllText( cpp_file_name, cpp_file_text.ToString() );
		}

		// Private interface
		private StringBuilder Build_Header_Text( string file_name )
		{
			StringBuilder header_text = new StringBuilder();

			Add_Top_Of_File_Comment( header_text, file_name );

			string ifdef_guard_name = Build_Ifdef_Guard();

			header_text.Append( "#ifndef " );
			header_text.Append( ifdef_guard_name );
			header_text.Append( END_OF_LINE );
			header_text.Append( "#define " );
			header_text.Append( ifdef_guard_name );
			header_text.Append( END_OF_LINE );
			header_text.Append( END_OF_LINE );

			header_text.Append( Build_Register_Function_Signature() );
			header_text.Append( ";" );
			header_text.Append( END_OF_LINE );
			header_text.Append( END_OF_LINE );

			header_text.Append( "#endif // " );
			header_text.Append( ifdef_guard_name );
			header_text.Append( END_OF_LINE );

			return header_text;
		}

		private StringBuilder Build_CPP_Text( string file_name )
		{
			StringBuilder cpp_text = new StringBuilder();

			Add_Top_Of_File_Comment( cpp_text, file_name );

			cpp_text.Append( "#include \"stdafx.h\"" );
			cpp_text.Append( END_OF_LINE );
			cpp_text.Append( END_OF_LINE );
			cpp_text.Append( "#include \"EnumConversion.h\"" );
			cpp_text.Append( END_OF_LINE );
			cpp_text.Append( END_OF_LINE );

			List< CEnumRecord > project_enums = new List< CEnumRecord >();
			CEnumReflector.EnumTracker.Build_Project_Enum_List( ID, project_enums );

			foreach ( var enum_record in project_enums )
			{
				if ( enum_record.Namespace.Length > 0 )
				{
					cpp_text.Append( "namespace " );
					cpp_text.Append( enum_record.Namespace );
					cpp_text.Append( END_OF_LINE );
					cpp_text.Append( "{" );
					cpp_text.Append( END_OF_LINE );
					cpp_text.Append( "\t" );
				}

				cpp_text.Append( "enum " );
				cpp_text.Append( enum_record.EnumName );
				cpp_text.Append( ";" );
				cpp_text.Append( END_OF_LINE );

				if ( enum_record.Namespace.Length > 0 )
				{
					cpp_text.Append( "}" );
					cpp_text.Append( END_OF_LINE );
				}
			}

			cpp_text.Append( END_OF_LINE );
			cpp_text.Append( Build_Register_Function_Signature() );
			cpp_text.Append( END_OF_LINE );
			cpp_text.Append( "{" );
			cpp_text.Append( END_OF_LINE );

			List< CEnumRecord > base_enums = new List< CEnumRecord >();

			foreach ( var enum_record in project_enums )
			{				
				Add_Enum_Conversions( cpp_text, enum_record );

				CEnumRecord base_enum = enum_record.BaseEnum;
				while ( base_enum != null )
				{
					Add_Secondary_Enum_Conversions( cpp_text, enum_record, base_enum );
					Add_Secondary_Enum_Conversions( cpp_text, base_enum, enum_record );

					base_enum = base_enum.BaseEnum;
				}
			}

			cpp_text.Append( "}" );
			cpp_text.Append( END_OF_LINE );

			return cpp_text;
		}

		private void Add_Enum_Conversions( StringBuilder cpp_text, CEnumRecord enum_record )
		{
			cpp_text.Append( "\tCEnumConverter::Register_Enum< " );
			cpp_text.Append( enum_record.FullName );
			cpp_text.Append( " >( \"" );
			cpp_text.Append( enum_record.FullName );
			cpp_text.Append( "\", " );
			if ( ( enum_record.Flags & EEnumFlags.IsBitfield ) != 0 )
			{
				cpp_text.Append( "CEP_BITFIELD" );
			}
			else
			{
				cpp_text.Append( "CEP_NONE" );
			}
			cpp_text.Append( " );" );
			cpp_text.Append( END_OF_LINE );

			foreach ( var entry in enum_record.Get_Entries() )
			{
				if ( entry.EntryName.Length > 0 )
				{
					cpp_text.Append( "\tCEnumConverter::Register_Enum_Entry( \"" );
					cpp_text.Append( entry.EntryName );
					cpp_text.Append( "\", static_cast< " );
					cpp_text.Append( enum_record.FullName );
					cpp_text.Append( " >( " );
					cpp_text.Append( entry.Value );
					cpp_text.Append( " ) );" );
					cpp_text.Append( END_OF_LINE );
				}
			}

			cpp_text.Append( END_OF_LINE );
		}

		private void Add_Secondary_Enum_Conversions( StringBuilder cpp_text, CEnumRecord secondary_enum, CEnumRecord primary_enum )
		{
			foreach ( var entry in secondary_enum.Get_Entries() )
			{
				if ( entry.EntryName.Length > 0 )
				{
					cpp_text.Append( "\tCEnumConverter::Register_Enum_Entry( \"" );
					cpp_text.Append( entry.EntryName );
					cpp_text.Append( "\", static_cast< " );
					cpp_text.Append( primary_enum.FullName );
					cpp_text.Append( " >( " );
					cpp_text.Append( entry.Value );
					cpp_text.Append( " ) );" );
					cpp_text.Append( END_OF_LINE );
				}
			}

			cpp_text.Append( END_OF_LINE );
		}

		private void Add_Top_Of_File_Comment( StringBuilder file_text, string file_name )
		{
			file_text.Append( "/**********************************************************************************************************************" );
			file_text.Append( END_OF_LINE );
			file_text.Append( END_OF_LINE );
			file_text.Append( "\t" );
			file_text.Append( file_name );
			file_text.Append( END_OF_LINE );
			file_text.Append( @"		A component that registers project-specific enum conversions.
		DO NOT EDIT THIS FILE; it is automatically generated.

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

**********************************************************************************************************************/" );

			file_text.Append( END_OF_LINE );
			file_text.Append( END_OF_LINE );
		}

		private void Parse_Project_File( FileInfo project_file )
		{
			XElement project_root = XElement.Load( project_file.FullName );
			string project_path = CEnumReflector.TopLevelDirectory + project_file.Directory.Name;

			foreach ( var elt in project_root.Elements().Where( e => e.Name.LocalName == "ItemGroup" ).
														 Elements().Where( e => e.Name.LocalName == "ClInclude" ).
														 Attributes().Where( a => a.Name.LocalName == "Include" ) )
			{
				string header_file_name = project_path + Path.DirectorySeparatorChar + elt.Value;

				string []split_directory = header_file_name.Split( Path.DirectorySeparatorChar );
				bool is_generated = false;
				for ( int i = 0; i + 1 < split_directory.Length; i++ )
				{
					if ( split_directory[ i ] == "GeneratedCode" )
					{
						is_generated = true;
						break;
					}
				}

				if ( is_generated )
				{
					continue;
				}

				FileInfo header_file_info = new FileInfo( header_file_name );
				CHeaderFileRecord new_header_record = new CHeaderFileRecord( header_file_name, NewProjectRecord.Name, header_file_info.LastWriteTime );
				
				CEnumReflector.HeaderFileTracker.Register_Header_File( ID, new_header_record );
			}
		}

		private string Build_Registration_Directory_Path()
		{
			return CEnumReflector.TopLevelDirectory + NewProjectRecord.CaseName + Path.DirectorySeparatorChar + "GeneratedCode" + Path.DirectorySeparatorChar;
		}

		private string Build_Registration_Header_File_Name()
		{
			return Build_Registration_Directory_Path() + "Register" + NewProjectRecord.CaseName + "Enums.h";
		}

		private string Build_Registration_CPP_File_Name()
		{
			return Build_Registration_Directory_Path() + "Register" + NewProjectRecord.CaseName + "Enums.cpp";
		}

		private string Build_Register_Function_Signature()
		{
			return "void Register_" + NewProjectRecord.CaseName + "_Enums( void )";
		}

		private string Build_Ifdef_Guard()
		{
			return "REGISTER_" + NewProjectRecord.Name + "_ENUMS_H";
		}

		// Properties
		public EProjectID ID { get; private set; }
		public string Name { get { return OldProjectRecord != null ? OldProjectRecord.Name : NewProjectRecord.Name; } }
		public EProjectCreationState CreationState { get; private set; }
		public EProjectState State { get; set; }

		public CProjectRecord OldProjectRecord { get; private set; }
		public CProjectRecord NewProjectRecord { get; private set; }

		// Constants
		private const string END_OF_LINE = "\r\n";
	}

	public class CProjectTracker
	{
		// Construction
		public CProjectTracker()
		{
		}

		// Methods
		// Public interface
		public void Initialize_DB_Projects()
		{
			foreach ( var project_record in CEnumXMLDatabase.Instance.Projects )
			{
				EProjectID id = Allocate_Project_ID();
				CProject project = new CProject( id, project_record );
				m_ProjectIDMap.Add( project.Name, id );
				m_Projects.Add( id, project );
			}
		}

		public void Initialize_File_Projects()
		{
			DirectoryInfo directory_info = new DirectoryInfo( CEnumReflector.TopLevelDirectory );

			foreach ( var subdirectory_info in directory_info.GetDirectories() )
			{
				foreach ( var file_info in subdirectory_info.GetFiles( "*.vcxproj" ) )
				{
					Register_Project( file_info );
				}
			}
		}

		public void Write_Enum_Registration_Files()
		{
			foreach ( var project in m_Projects.Values )
			{	
				if ( project.CreationState == EProjectCreationState.New || project.State == EProjectState.Dirty )
				{
					project.Write_Enum_Registration_Files();
				}
			}
		}

		public CProject Get_Project_By_ID( EProjectID id )
		{
			return m_Projects[ id ];
		}

		public CProject Get_Project_By_Name( string project_name )
		{
			EProjectID project_id = EProjectID.Invalid;
			if ( !m_ProjectIDMap.TryGetValue( project_name.ToUpper(), out project_id ) )
			{
				return null;
			}

			return Get_Project_By_ID( project_id );
		}

		// Private interface
		private bool Should_Skip_Project( string project_name )
		{
			for ( int i = 0; i < SKIPPED_PROJECTS.Length; ++i )
			{
				if ( project_name == SKIPPED_PROJECTS[ i ] )
				{
					return true;
				}
			}

			return false;
		}

		private void Register_Project( FileInfo project_file )
		{
			string project_name = Path.GetFileNameWithoutExtension( project_file.Name );
			string upper_project_name = project_name.ToUpper();

			if ( Should_Skip_Project( upper_project_name ) )
			{
				return;
			}

			CLogInterface.Write_Line( "Found project: " + project_name );
			CProject existing_project = Get_Project_By_Name( upper_project_name );
			if ( existing_project != null )
			{
				existing_project.Initialize_Existing( project_file );
			}
			else
			{
				EProjectID id = Allocate_Project_ID();
				CProject project = new CProject( id, project_file );
				m_ProjectIDMap.Add( upper_project_name, id );
				m_Projects.Add( id, project );
			}
		}

		private EProjectID Allocate_Project_ID()
		{
			return m_NextAllocatedID++;
		}

		// Properties
		public IEnumerable< CProjectRecord > SaveRecords { get { return m_Projects.Values.Where( p => p.NewProjectRecord != null ).Select( p => p.NewProjectRecord ); } }

		// Fields
		private Dictionary< EProjectID, CProject > m_Projects = new Dictionary< EProjectID, CProject >();
		private Dictionary< string, EProjectID > m_ProjectIDMap = new Dictionary< string, EProjectID >();
		private EProjectID m_NextAllocatedID = EProjectID.Invalid + 1;

		private static string[] SKIPPED_PROJECTS = { "GTEST-MD", "PLATFORM", "PLATFORMTEST", "PUGIXML" };
	}
}