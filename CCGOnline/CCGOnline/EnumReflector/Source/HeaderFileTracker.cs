using System;
using System.IO;
using System.Collections.Generic;

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

		Unknown,
		Dirty,
		Unchanged
	}

	public class CHeaderFile
	{
		public CHeaderFile( EHeaderFileID id, CHeaderFileRecord old_header_file_record )
		{
			ID = id;
			OldHeaderFileRecord = old_header_file_record;
			CreationState = EHeaderFileCreationState.Deleted;
			State = EHeaderFileState.Invalid;
		} 
/*
		public CHeaderFile( EHeaderFileID id, FileInfo header_file )
		{
			ID = id;
			NewHeaderFileRecord = new CHeaderFileRecord( ?? );
			CreationState = EHeaderFileCreationState.New;
			State = EHeaderFileState.Dirty;
		} 

		public void Initialize_Existing( FileInfo header_file )
		{
			NewHeaderFileRecord = new CHeaderFileRecord( ?? );
			CreationState = EHeaderFileCreationState.Unchanged;
			State = EHeaderFileState.Unknown;
		}
*/
		public EHeaderFileID ID { get; private set; }
		public string FileNameWithPath { get { return OldHeaderFileRecord != null ? OldHeaderFileRecord.FileNameWithPath : NewHeaderFileRecord.FileNameWithPath; } }
//		public string Name { get { return OldHeaderFileRecord != null ? OldHeaderFileRecord.Name : NewHeaderFileRecord.Name; } }
		public EHeaderFileCreationState CreationState { get; private set; }
		public EHeaderFileState State { get; private set; }

		private CHeaderFileRecord OldHeaderFileRecord { get; set; }
		private CHeaderFileRecord NewHeaderFileRecord { get; set; }
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

		public void Initialize_File_Header_Files()
		{
		}

		public CHeaderFile Get_Header_File_By_ID( EHeaderFileID id )
		{
			return m_HeaderFiles[ id ];
		}

/*
		private void Register_Project( FileInfo project_file )
		{
			string project_name = Path.GetFileNameWithoutExtension( project_file.Name );
			string upper_project_name = project_name.ToUpper();

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
*/

		private EHeaderFileID Allocate_Header_File_ID()
		{
			return m_NextAllocatedID++;
		}

		private CHeaderFile Get_Header_File_By_Name( string header_file_name )
		{
			EHeaderFileID id = EHeaderFileID.Invalid;
			if ( !m_HeaderFileIDMap.TryGetValue( header_file_name, out id ) )
			{
				return null;
			}

			return Get_Header_File_By_ID( id );
		}

		private Dictionary< EHeaderFileID, CHeaderFile > m_HeaderFiles = new Dictionary< EHeaderFileID, CHeaderFile >();
		private Dictionary< string, EHeaderFileID > m_HeaderFileIDMap = new Dictionary< string, EHeaderFileID >();
		private EHeaderFileID m_NextAllocatedID = EHeaderFileID.Invalid + 1;
	}
}