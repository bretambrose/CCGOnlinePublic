using System;
using System.IO;
using System.Collections.Generic;
using System.Linq;

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
			NewEnumRecord = null;
			CreationState = EEnumCreationState.Deleted;
			State = EEnumState.Invalid;
		} 

		public CEnum( EEnumID id, EHeaderFileID header_id, CEnumRecord new_record )
		{
			ID = id;
			OldEnumRecord = null;
			NewEnumRecord = new_record;
			CreationState = EEnumCreationState.New;
			State = EEnumState.Dirty;
		} 

		public void Initialize_Existing( EHeaderFileID header_id, CEnumRecord new_record )
		{
			OldEnumRecord.HeaderFileID = header_id;
			NewEnumRecord = new_record;
			CreationState = EEnumCreationState.Unchanged;
			State = NewEnumRecord.Value_Equals( OldEnumRecord ) ? EEnumState.Unchanged : EEnumState.Dirty;
		}

		public void Set_Old_Header_ID( EHeaderFileID id )
		{
			OldEnumRecord.HeaderFileID = id;
		}

		public EEnumID ID { get; private set; }
		public string Name { get { return OldEnumRecord != null ? OldEnumRecord.Name : NewEnumRecord.Name; } }
		public string HeaderFileName { get { return OldEnumRecord != null ? OldEnumRecord.HeaderFileName : NewEnumRecord.HeaderFileName; } }
		public EEnumCreationState CreationState { get; private set; }
		public EEnumState State { get; set; }

		public CEnumRecord OldEnumRecord { get; private set; }
		public CEnumRecord NewEnumRecord { get; private set; }
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

		public void Initialize_Starting_Enum_States()
		{
			foreach ( var enum_pair in m_Enums )
			{
				CHeaderFile header_file = CEnumReflector.HeaderFileTracker.Get_Header_ID_By_File_Path( enum_pair.Value.HeaderFileName );
				enum_pair.Value.Set_Old_Header_ID( header_file.ID );

				if ( header_file.State == EHeaderFileState.Unchanged )
				{
					enum_pair.Value.State = EEnumState.Unchanged;
				}
				else if ( header_file.State == EHeaderFileState.Dirty || header_file.CreationState == EHeaderFileCreationState.Deleted )
				{
					enum_pair.Value.State = EEnumState.Unknown;
				}
				else
				{
					throw new Exception( "Invalid header file state" );
				}
			}
		}

		public void Process_Final_States()
		{
			foreach ( var en in m_Enums.Values )
			{
				if ( en.CreationState == EEnumCreationState.Unchanged && en.NewEnumRecord.Value_Equals( en.OldEnumRecord ) )
				{
					continue;
				}

				if ( en.OldEnumRecord != null )
				{
					Mark_Owning_Project_Dirty( en.OldEnumRecord.HeaderFileID );
				}

				if ( en.NewEnumRecord != null )
				{
					Mark_Owning_Project_Dirty( en.NewEnumRecord.HeaderFileID );
				}
			}
		}

		public void Build_Project_Enum_List( EProjectID project_id, List< CEnumRecord > enum_records )
		{
			foreach ( var record in m_Enums.Values.Select( en => en.NewEnumRecord ) )
			{
				if ( record == null )
				{
					continue;
				}

				CHeaderFile header_file = CEnumReflector.HeaderFileTracker.Get_Header_File_By_ID( record.HeaderFileID );
				if ( header_file == null )
				{
					continue;
				}

				if ( header_file.ProjectID != project_id )
				{
					continue;
				}

				enum_records.Add( record );
			}
		}

		public CEnum Get_Enum_By_ID( EEnumID id )
		{
			return m_Enums[ id ];
		}

		private void Mark_Owning_Project_Dirty( EHeaderFileID header_id )
		{
			CHeaderFile header_file = CEnumReflector.HeaderFileTracker.Get_Header_File_By_ID( header_id );
			CProject project = CEnumReflector.ProjectTracker.Get_Project_By_ID( header_file.ProjectID );

			project.State = EProjectState.Dirty;
		}

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

		// Properties
		public IEnumerable< CEnumRecord > SaveRecords { get { return m_Enums.Values.Where( e => e.NewEnumRecord != null ).Select( e => e.NewEnumRecord ); } }

		// Fields
		private Dictionary< EEnumID, CEnum > m_Enums = new Dictionary< EEnumID, CEnum >();
		private Dictionary< string, EEnumID > m_EnumIDMap = new Dictionary< string, EEnumID >();
		private EEnumID m_NextAllocatedID = EEnumID.Invalid + 1;
	}

}