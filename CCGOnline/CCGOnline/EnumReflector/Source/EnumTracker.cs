using System;
using System.IO;
using System.Collections.Generic;

namespace EnumReflector
{
	public enum EEnumID
	{
		Invalid = 0
	}

	public enum EEnumCreationState
	{
		Invalid = 0,

		New,
		Deleted,
		Unchanged
	}

	public enum EEnumState
	{
		Invalid = 0,

		Unknown,
		Dirty,
		Unchanged
	}

	public class CEnum
	{
		public CEnum( EEnumID id, CEnumRecord old_enum_record )
		{
			ID = id;
			OldEnumRecord = old_enum_record;
			CreationState = EEnumCreationState.Deleted;
			State = EEnumState.Invalid;
		} 
/*
		public CEnum( EEnumID id, ?? )
		{
			ID = id;
			NewEnumRecord = new CEnumRecord( ?? );
			CreationState = EEnumCreationState.New;
			State = EEnumState.Dirty;
		} 

		public void Initialize_Existing( ?? )
		{
			NewEnumRecord = new CEnumRecord( ?? );
			CreationState = EEnumCreationState.Unchanged;
			State = EEnumState.Unknown;
		}
*/
		public EEnumID ID { get; private set; }
		public string Name { get { return OldEnumRecord != null ? OldEnumRecord.Name : NewEnumRecord.Name; } }
		public EEnumCreationState CreationState { get; private set; }
		public EEnumState State { get; private set; }

		private CEnumRecord OldEnumRecord { get; set; }
		private CEnumRecord NewEnumRecord { get; set; }
	}

	public class CEnumTracker
	{
		public CEnumTracker()
		{
		}

		public void Initialize_DB_Enums()
		{
			foreach ( var enum_record in CEnumXMLDatabase.Instance.Enums )
			{
				EEnumID id = Allocate_Enum_ID();
				CEnum enum_instance = new CEnum( id, enum_record );
				m_EnumIDMap.Add( enum_instance.Name, id );
				m_Enums.Add( id, enum_instance );
			}
		}

		public void Initialize_File_Enums()
		{
		}

		public CEnum Get_Enum_By_ID( EEnumID id )
		{
			return m_Enums[ id ];
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

		private EEnumID Allocate_Enum_ID()
		{
			return m_NextAllocatedID++;
		}

		private CEnum Get_Enum_By_Name( string enum_name )
		{
			EEnumID id = EEnumID.Invalid;
			if ( !m_EnumIDMap.TryGetValue( enum_name, out id ) )
			{
				return null;
			}

			return Get_Enum_By_ID( id );
		}

		private Dictionary< EEnumID, CEnum > m_Enums = new Dictionary< EEnumID, CEnum >();
		private Dictionary< string, EEnumID > m_EnumIDMap = new Dictionary< string, EEnumID >();
		private EEnumID m_NextAllocatedID = EEnumID.Invalid + 1;
	}

}