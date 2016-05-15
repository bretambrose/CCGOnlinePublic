/**********************************************************************************************************************

	HeaderFileTracker.cs		
 		A pair of classes for tracking the before (cached in an XML "DB") and after (header file read and parsed)
		states of a C++ header file.  Unlike enums, before and after doesn't make a whole lot of sense here
		but I've kept it for symmetry preservation.
 
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
using System.Linq;
using System.Text;

namespace EnumReflector
{
	public enum EHeaderFileID
	{
		Invalid = 0
	}

	public enum EHeaderFileCreationState
	{
		Invalid = 0,

		New,
		Deleted,
		Unchanged
	}

	public enum EHeaderFileState
	{
		Invalid = 0,

		Dirty,
		Unchanged
	}

	public class CHeaderFile
	{
		// Construction
		public CHeaderFile( EHeaderFileID id, CHeaderFileRecord old_header_file_record )
		{
			ID = id;
			ProjectID = EProjectID.Invalid;
			OldHeaderFileRecord = old_header_file_record;
			NewHeaderFileRecord = null;
			CreationState = EHeaderFileCreationState.Deleted;
			State = EHeaderFileState.Invalid;
		} 

		public CHeaderFile( EHeaderFileID id, EProjectID project_id, CHeaderFileRecord new_record )
		{
			ID = id;
			ProjectID = project_id;
			OldHeaderFileRecord = null;
			NewHeaderFileRecord = new_record;
			CreationState = EHeaderFileCreationState.New;
			State = EHeaderFileState.Dirty;
		} 

		// Methods
		// Public interface
		public void Initialize_Existing( EProjectID project_id, CHeaderFileRecord new_record )
		{
			NewHeaderFileRecord = new_record;
			CreationState = EHeaderFileCreationState.Unchanged;
			State = ( NewHeaderFileRecord.LastModifiedTime > OldHeaderFileRecord.LastModifiedTime ) ? EHeaderFileState.Dirty : EHeaderFileState.Unchanged;
			ProjectID = project_id;
		}

		public void Set_Project_ID( EProjectID project_id )
		{
			ProjectID = project_id;
		}

		public void Reparse_Enums()
		{
			CLogInterface.Write_Line( "Parsing file: " + NewHeaderFileRecord.FileNameWithPath );
			using ( FileStream fs = File.Open( NewHeaderFileRecord.FileNameWithPath, FileMode.Open ) )
			using ( TextReader tr = new StreamReader( fs ) )
			{
				bool in_enum_definition = false;
				string header_string = tr.ReadToEnd();
				int current_line_start = 0;
				int current_line_end = 0;
				int definition_start = 0;
				int definition_end = 0;
				while ( current_line_start < header_string.Length )
				{
					int current_line_pos = Skip_Line_Whitespace( header_string, current_line_start );
					current_line_end = Find_Current_Line_End( header_string, current_line_start );

					if ( Substring_Compare( header_string, current_line_pos, REFLECTION_DIRECTIVE ) )
					{
						current_line_pos = Skip_Line_Whitespace( header_string, current_line_pos + 3 );
						if ( Substring_Compare( header_string, current_line_pos, ENUM_PREFIX ) )
						{
							current_line_pos += 4;
							if ( Substring_Compare( header_string, current_line_pos, ENUM_BEGIN_DIRECTIVE ) )
							{
								if ( in_enum_definition )
								{
									throw new Exception( "Processed two consecutive EnumBegin directives." );
								}
								else
								{
									in_enum_definition = true;
									definition_start = current_line_start;
								}
							}
							else if ( Substring_Compare( header_string, current_line_pos, ENUM_END_DIRECTIVE ) )
							{
								if ( !in_enum_definition )
								{
									throw new Exception( "Processed two consecutive EnumEnd directives." );
								}
								else
								{
									in_enum_definition = false;
									definition_end = current_line_end;

									// with the enum definition split out, parse it and analyze the AST in order to extract all the needed information
									string parse_string = header_string.Substring( definition_start, definition_end - definition_start + 1 );
									CEnumRecord new_record = CEnumASTUtils.Parse_Enum_Definition( parse_string, NewHeaderFileRecord.FileNameWithPath );

									CEnumReflector.EnumTracker.Initialize_Parsed_Enum( new_record );
								}
							}
						}
					}

					current_line_start = current_line_end + 1;
					while ( current_line_start < header_string.Length )
					{
						char current_char = header_string[ current_line_start ];
						if ( current_char != '\n' && current_char != '\r' )
						{
							break;
						}

						current_line_start++;
					}
				}

				if ( in_enum_definition )
				{
					throw new Exception( "Enum reflection directive was not properly closed" );
				}
			}
		}

		// Private interface
		private int Skip_Line_Whitespace( string value, int index )
		{
			char current_char = value[ index ];
			while ( current_char == '\t' || current_char == ' ' && index + 1 < value.Length )
			{
				current_char = value[ ++index ];
			}

			return index;
		}

		private int Find_Current_Line_End( string value, int index )
		{
			char current_char = value[ index ];
			while ( current_char != '\r' && current_char != '\n' && index + 1 < value.Length )
			{
				current_char = value[ ++index ];
			}

			return index;
		}

		private bool Substring_Compare( string base_string, int index, string compare_string )
		{
			int compare_string_length = compare_string.Length;

			if ( index + compare_string_length > base_string.Length )
			{
				return false;
			}

			for ( int i = 0; i < compare_string_length; i++ )
			{
				if ( base_string[ i + index ] != compare_string[ i ] )
				{
					return false;
				}
			}

			return true;
		}

		// Properties
		public EHeaderFileID ID { get; private set; }
		public EProjectID ProjectID { get; private set; }
		public string FileNameWithPath { get { return OldHeaderFileRecord != null ? OldHeaderFileRecord.FileNameWithPath : NewHeaderFileRecord.FileNameWithPath; } }
		public EHeaderFileCreationState CreationState { get; private set; }
		public EHeaderFileState State { get; private set; }

		public CHeaderFileRecord OldHeaderFileRecord { get; private set; }
		public CHeaderFileRecord NewHeaderFileRecord { get; private set; }

		// Constants
		private const string REFLECTION_DIRECTIVE = @"//:";
		private const string ENUM_PREFIX = @"Enum";
		private const string ENUM_BEGIN_DIRECTIVE = @"Begin";
		private const string ENUM_END_DIRECTIVE = @"End";
	}

	public class CHeaderFileTracker
	{
		// Construction
		public CHeaderFileTracker()
		{
		}

		// Methods
		// Public Interface
		public void Initialize_DB_Header_Files()
		{
			foreach ( var header_file_record in CEnumXMLDatabase.Instance.HeaderFiles )
			{
				EHeaderFileID id = Allocate_Header_File_ID();
				CHeaderFile header_file = new CHeaderFile( id, header_file_record );
				m_HeaderFileIDMap.Add( header_file.FileNameWithPath, id );
				m_HeaderFiles.Add( id, header_file );
			}
		}

		public CHeaderFile Get_Header_File_By_ID( EHeaderFileID id )
		{
			CHeaderFile header_file = null;
			m_HeaderFiles.TryGetValue( id, out header_file );

			return header_file;
		}

		public void Register_Header_File( EProjectID project_id, CHeaderFileRecord header_record )
		{
			EHeaderFileID header_id = EHeaderFileID.Invalid;
			if ( m_HeaderFileIDMap.TryGetValue( header_record.FileNameWithPath, out header_id ) )
			{
				CHeaderFile header_file = null;
				if ( !m_HeaderFiles.TryGetValue( header_id, out header_file ) )
				{
					throw new Exception( "Internal header file mapping error" );
				}

				header_file.Initialize_Existing( project_id, header_record );
			}
			else
			{
				header_id = Allocate_Header_File_ID();
				CHeaderFile header_file = new CHeaderFile( header_id, project_id, header_record );
				m_HeaderFileIDMap.Add( header_file.FileNameWithPath, header_id );
				m_HeaderFiles.Add( header_id, header_file );
			}
		}

		public void Initialize_Starting_Header_Projects()
		{
			foreach ( var header_file in m_HeaderFiles.Values )
			{
				string project_name = header_file.NewHeaderFileRecord != null ? header_file.NewHeaderFileRecord.Project : header_file.OldHeaderFileRecord.Project;

				CProject project = CEnumReflector.ProjectTracker.Get_Project_By_Name( project_name );
				header_file.Set_Project_ID( project.ID );
			}
		}

		public void Process_Dirty_Headers()
		{
			foreach ( var header in m_HeaderFiles.Values )
			{
				if ( header.CreationState == EHeaderFileCreationState.New || header.State == EHeaderFileState.Dirty )
				{
					header.Reparse_Enums();
				}
			}
		}

		public CHeaderFile Get_Header_ID_By_File_Path( string header_file_path )
		{
			EHeaderFileID id = EHeaderFileID.Invalid;
			if ( !m_HeaderFileIDMap.TryGetValue( header_file_path, out id ) )
			{
				return null;
			}

			return Get_Header_File_By_ID( id );
		}

		// Private interface
		private EHeaderFileID Allocate_Header_File_ID()
		{
			return m_NextAllocatedID++;
		}

		// Properties
		public IEnumerable< CHeaderFileRecord > SaveRecords { get { return m_HeaderFiles.Values.Where( hf => hf.NewHeaderFileRecord != null ).Select( hf => hf.NewHeaderFileRecord ); } }

		// Fields
		private Dictionary< EHeaderFileID, CHeaderFile > m_HeaderFiles = new Dictionary< EHeaderFileID, CHeaderFile >();
		private Dictionary< string, EHeaderFileID > m_HeaderFileIDMap = new Dictionary< string, EHeaderFileID >();
		private EHeaderFileID m_NextAllocatedID = EHeaderFileID.Invalid + 1;
	}
}