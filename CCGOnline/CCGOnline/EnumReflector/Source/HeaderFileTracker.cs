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

		public void Initialize_Existing( EProjectID project_id, CHeaderFileRecord new_record )
		{
			NewHeaderFileRecord = new_record;
			CreationState = EHeaderFileCreationState.Unchanged;
			State = ( NewHeaderFileRecord.LastModifiedTime > OldHeaderFileRecord.LastModifiedTime ) ? EHeaderFileState.Dirty : EHeaderFileState.Unchanged;
			ProjectID = project_id;
		}

		public void Reparse_Enums()
		{
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
							else if ( Substring_Compare( header_string, current_line_start, ENUM_END_DIRECTIVE ) )
							{
								if ( !in_enum_definition )
								{
									throw new Exception( "Processed two consecutive EnumEnd directives." );
								}
								else
								{
									in_enum_definition = false;
									definition_end = current_line_end;

									string parse_string = header_string.Substring( definition_start, definition_end - definition_start + 1 );
									CEnumASTUtils.Parse_Enum_Definition( parse_string, NewHeaderFileRecord.FileName );
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
			}
		}

		private int Skip_Line_Whitespace( string value, int index )
		{
			char current_char = value[ index ];
			while ( current_char == '\t' || current_char == ' ' )
			{
				current_char = value[ ++index ];
			}

			return index;
		}

		private int Find_Current_Line_End( string value, int index )
		{
			char current_char = value[ index ];
			while ( current_char != '\r' && current_char != '\n' )
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

		public EHeaderFileID ID { get; private set; }
		public EProjectID ProjectID { get; private set; }
		public string FileNameWithPath { get { return OldHeaderFileRecord != null ? OldHeaderFileRecord.FileNameWithPath : NewHeaderFileRecord.FileNameWithPath; } }
		public EHeaderFileCreationState CreationState { get; private set; }
		public EHeaderFileState State { get; private set; }

		public CHeaderFileRecord OldHeaderFileRecord { get; private set; }
		public CHeaderFileRecord NewHeaderFileRecord { get; private set; }

		private const string REFLECTION_DIRECTIVE = @"//:";
		private const string ENUM_PREFIX = @"Enum";
		private const string ENUM_BEGIN_DIRECTIVE = @"Begin";
		private const string ENUM_END_DIRECTIVE = @"End";
	}

	public class CHeaderFileTracker
	{
		public CHeaderFileTracker()
		{
		}

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
			return m_HeaderFiles[ id ];
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