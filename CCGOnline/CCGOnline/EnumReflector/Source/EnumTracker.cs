/**********************************************************************************************************************

	EnumTracker.cs
		A pair of classes for tracking the before (cached in an XML "DB") and after (header file read and parsed)
		states of a tagged enum.

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
		// Construction
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

		// Methods
		// Public interface
		public void Initialize_Existing( CEnumRecord new_record )
		{
			NewEnumRecord = new_record;
			CreationState = EEnumCreationState.Unchanged;
			State = NewEnumRecord.Value_Equals( OldEnumRecord ) ? EEnumState.Unchanged : EEnumState.Dirty;
		}

		public void Set_Old_Header_ID( EHeaderFileID id )
		{
			OldEnumRecord.HeaderFileID = id;
		}

		public void Promote_To_Unchanged()
		{
			if ( NewEnumRecord != null || CreationState != EEnumCreationState.Deleted )
			{
				throw new Exception( "Illegal promotion to unchanged state for enum " + Name );
			}

			CreationState = EEnumCreationState.Unchanged;
			NewEnumRecord = OldEnumRecord;	// ugly but safe, avoids having to write a clone
		}

		// Properties
		public EEnumID ID { get; private set; }
		public string Name { get { return OldEnumRecord != null ? OldEnumRecord.Name : NewEnumRecord.Name; } }
		public string FileNameWithPath { get { return OldEnumRecord != null ? OldEnumRecord.FileNameWithPath : NewEnumRecord.FileNameWithPath; } }
		public EEnumCreationState CreationState { get; private set; }
		public EEnumState State { get; set; }

		public CEnumRecord OldEnumRecord { get; private set; }
		public CEnumRecord NewEnumRecord { get; private set; }
	}

	public class CEnumTracker
	{
		// Construction
		public CEnumTracker()
		{
		}

		// Methods
		// Private interface
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

		public void Initialize_Parsed_Enum( CEnumRecord record )
		{
			EEnumID id = EEnumID.Invalid;
			if ( m_EnumIDMap.TryGetValue( record.Name, out id ) )
			{
				CEnum enum_instance = null;
				if ( !m_Enums.TryGetValue( id, out enum_instance ) )
				{
					throw new Exception( "Internal error: enum id mapping exists, but not enum instance could be found" );
				}

				enum_instance.Initialize_Existing( record );
			}
			else
			{
				id = Allocate_Enum_ID();
				CEnum enum_instance = new CEnum( id, record.HeaderFileID, record );
				m_EnumIDMap.Add( enum_instance.Name, id );
				m_Enums.Add( id, enum_instance );
			}
		}
		
		public void Initialize_Starting_Enum_States()
		{
			foreach ( var enum_pair in m_Enums )
			{
				CHeaderFile header_file = CEnumReflector.HeaderFileTracker.Get_Header_ID_By_File_Path( enum_pair.Value.FileNameWithPath );
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
				// deleted is the default state for existing enums; if the header the enum is defined in is unchanged, then this
				// enum didn't change
				if ( en.CreationState == EEnumCreationState.Deleted )
				{
					CHeaderFile header_file = CEnumReflector.HeaderFileTracker.Get_Header_File_By_ID( en.OldEnumRecord.HeaderFileID );
					if ( header_file != null && header_file.State == EHeaderFileState.Unchanged )
					{
						en.Promote_To_Unchanged();
					}
				}

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

		// Private interface
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