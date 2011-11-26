using System;
using System.IO;
using System.Collections.Generic;

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
		public CProject( EProjectID id, CProjectRecord old_project_record )
		{
			ID = id;
			OldProjectRecord = old_project_record;
			CreationState = EProjectCreationState.Deleted;
			State = EProjectState.Invalid;
		} 

		public CProject( EProjectID id, FileInfo project_file )
		{
			ID = id;
			NewProjectRecord = new CProjectRecord( Path.GetFileNameWithoutExtension( project_file.FullName ) );
			CreationState = EProjectCreationState.New;
			State = EProjectState.Dirty;
		} 

		public void Initialize_Existing( FileInfo project_file )
		{
			NewProjectRecord = new CProjectRecord( Path.GetFileNameWithoutExtension( project_file.FullName ) );
			CreationState = EProjectCreationState.Unchanged;
			State = EProjectState.Unknown;
		}

		public EProjectID ID { get; private set; }
		public string Name { get { return OldProjectRecord != null ? OldProjectRecord.Name : NewProjectRecord.Name; } }
		public EProjectCreationState CreationState { get; private set; }
		public EProjectState State { get; private set; }

		private CProjectRecord OldProjectRecord { get; set; }
		private CProjectRecord NewProjectRecord { get; set; }
	}

	public class CProjectTracker
	{
		public CProjectTracker()
		{
		}

		public void Initialize_Project_Set()
		{
			foreach ( var project_record in CEnumXMLDatabase.Instance.Projects )
			{
				EProjectID id = Allocate_Project_ID();
				CProject project = new CProject( id, project_record );
				m_ProjectIDMap.Add( project.Name, id );
				m_Projects.Add( id, project );
			}

			DirectoryInfo directory_info = new DirectoryInfo( CEnumReflector.TopLevelDirectory );

			foreach ( var subdirectory_info in directory_info.GetDirectories() )
			{
				foreach ( var file_info in subdirectory_info.GetFiles( "*.vsproj" ) )
				{
					Register_Project( file_info );
				}
			}
		}

		public CProject Get_Project_By_ID( EProjectID id )
		{
			return m_Projects[ id ];
		}

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

		private EProjectID Allocate_Project_ID()
		{
			return m_NextAllocatedID++;
		}

		private CProject Get_Project_By_Name( string project_name )
		{
			EProjectID project_id = EProjectID.Invalid;
			if ( !m_ProjectIDMap.TryGetValue( project_name, out project_id ) )
			{
				return null;
			}

			return Get_Project_By_ID( project_id );
		}

		private Dictionary< EProjectID, CProject > m_Projects = new Dictionary< EProjectID, CProject >();
		private Dictionary< string, EProjectID > m_ProjectIDMap = new Dictionary< string, EProjectID >();
		private EProjectID m_NextAllocatedID = EProjectID.Invalid + 1;
	}
}