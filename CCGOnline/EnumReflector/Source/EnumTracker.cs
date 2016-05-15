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
			NewEnumRecord = OldEnumRecord.Clone();
		}

		// Properties
		public EEnumID ID { get; private set; }
		public string Name { get { return OldEnumRecord != null ? OldEnumRecord.FullName : NewEnumRecord.FullName; } }
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
				m_EnumIDMap.Add( enum_record.FullName, id );
				m_Enums.Add( id, enum_instance );
			}
		}

		public void Initialize_Parsed_Enum( CEnumRecord record )
		{
			EEnumID id = EEnumID.Invalid;
			if ( m_EnumIDMap.TryGetValue( record.FullName, out id ) )
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
				m_EnumIDMap.Add( record.FullName, id );
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
			}

			CLogInterface.Write_Line( "Setting base enums" );
			Set_Base_Enums();

			CLogInterface.Write_Line( "Binding derived enum entries" );
			Bind_Derived_Enums();

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

		public void Build_Referenced_Enum_Set( List< CEnumRecord > project_enums, HashSet< CEnumRecord > referenced_enums )
		{
			foreach( var enum_record in project_enums )
			{
				referenced_enums.Add( enum_record );

				CEnumRecord base_record = enum_record.BaseEnum;
				while ( base_record != null )
				{
					referenced_enums.Add( base_record );
					base_record = base_record.BaseEnum;
				}
			}
		}

		public CEnum Get_Enum_By_ID( EEnumID id )
		{
			return m_Enums[ id ];
		}

		// Private interface
		private void Set_Base_Enums()
		{
			foreach ( var en in m_Enums.Values )
			{
				CEnumRecord derived_record = en.NewEnumRecord;
				if ( derived_record == null )
				{
					continue;
				}

				if ( derived_record.ExtendsEnum.Length > 0 )
				{
					CEnum base_enum = Get_Enum_By_Full_Name( derived_record.ExtendsEnum );
					if ( base_enum == null )
					{
						throw new Exception( "Enum " + derived_record.FullName + " is an extension of an unknown enum: " + derived_record.ExtendsEnum );
					}

					CEnumRecord base_record = base_enum.NewEnumRecord;
					if ( base_record == null )
					{
						throw new Exception( "Enum " + derived_record.FullName + " is an extension of a non-existent enum: " + derived_record.ExtendsEnum );
					}

					if ( base_record == derived_record )
					{
						throw new Exception( "Enum " + derived_record.FullName + " is an extension of itself" );
					}

					bool is_derived_bitfield = derived_record.Is_Bitfield();
					bool is_base_bitfield = base_record.Is_Bitfield();
					if ( is_base_bitfield != is_derived_bitfield )
					{
						throw new Exception( "IsBitfield mismatch between derived enum " + derived_record.FullName + " and base enum " + base_record.FullName );
					}

					derived_record.BaseEnum = base_record;
				}
			}
		}

		private void Bind_Derived_Enums()
		{
			bool made_a_change = true;
			while ( made_a_change )
			{
				made_a_change = false;

				foreach ( var en in m_Enums.Values )
				{
					CEnumRecord new_record = en.NewEnumRecord;
					if ( new_record == null )
					{
						continue;
					}

					if ( new_record.ExtensionInitialized )
					{
						continue;
					}

					if ( new_record.BaseEnum == null )
					{
						new_record.ExtensionInitialized = true;
						made_a_change = true;
						continue;
					}

					CEnumRecord base_record = en.NewEnumRecord.BaseEnum;
					if ( !base_record.ExtensionInitialized )
					{
						continue;
					}
						
					Compute_Extension_Enum_Starting_Value( new_record, base_record );
					new_record.Bind_Unbound_Values();

					made_a_change = true;
				}
			}

			foreach ( var en in m_Enums.Values )
			{
				if ( en.NewEnumRecord != null && !en.NewEnumRecord.ExtensionInitialized )
				{
					throw new Exception( "Enum " + en.NewEnumRecord.FullName + " still has unbound extension values after processing completed." );
				}
			}
		}

		private void Compute_Extension_Enum_Starting_Value( CEnumRecord derived_record, CEnumRecord base_record )
		{
			CEnumEntry first_entry = derived_record.Get_Entries().First();
			if ( first_entry == null || first_entry.BoundName.Length == 0 )
			{
				throw new Exception( "Derived enum " + derived_record.FullName + " needs a first entry that assigns a value from the base enum " + base_record.FullName );
			}

			CEnumEntry base_entry = base_record.Get_Entry_By_CPP_Name( first_entry.BoundName );
			if ( base_entry == null )
			{
				throw new Exception( "Derived enum " + derived_record.FullName + " has a first entry that references " + first_entry.BoundName + " in enum " + base_record.FullName + " but that entry does not exist" );
			}

			derived_record.StartingValue = base_entry.Value;
		}

		private void Mark_Owning_Project_Dirty( EHeaderFileID header_id )
		{
			CHeaderFile header_file = CEnumReflector.HeaderFileTracker.Get_Header_File_By_ID( header_id );
			CProject project = CEnumReflector.ProjectTracker.Get_Project_By_ID( header_file.ProjectID );

			CLogInterface.Write_Line( "Marking project " + project.Name + " dirty due to header file " + header_file.FileNameWithPath );

			project.State = EProjectState.Dirty;
		}

		private EEnumID Allocate_Enum_ID()
		{
			return m_NextAllocatedID++;
		}

		private CEnum Get_Enum_By_Full_Name( string enum_full_name )
		{
			EEnumID id = EEnumID.Invalid;
			if ( !m_EnumIDMap.TryGetValue( enum_full_name, out id ) )
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