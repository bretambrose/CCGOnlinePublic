using System;
using System.IO;
using System.Collections.Generic;
using System.Linq;

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

		public EHeaderFileID ID { get; private set; }
		public EProjectID ProjectID { get; private set; }
		public string FileNameWithPath { get { return OldHeaderFileRecord != null ? OldHeaderFileRecord.FileNameWithPath : NewHeaderFileRecord.FileNameWithPath; } }
		public EHeaderFileCreationState CreationState { get; private set; }
		public EHeaderFileState State { get; private set; }

		public CHeaderFileRecord OldHeaderFileRecord { get; private set; }
		public CHeaderFileRecord NewHeaderFileRecord { get; private set; }
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