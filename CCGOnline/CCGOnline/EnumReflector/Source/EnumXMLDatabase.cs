/**********************************************************************************************************************

	EnumXMLDatabase.cs
		A wrapper class for reading and writing (from/to XML) all the project file, header file, and enum definition data needed
		in order to efficiently and correctly incrementally update and emit the auto-generated enum conversion code.

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
using System.Collections.Generic;
using System.Text;
using System.Runtime.Serialization;
using System.Xml.Serialization;
using System.Xml;
using System.IO;
using System.Net;

namespace EnumReflector
{
	[ DataContract( Name="HeaderFile", Namespace="http://www.bretambrose.com" ) ]
	public sealed class CHeaderFileRecord
	{
		// Construction
		public CHeaderFileRecord()
		{
			Project = String.Empty;
			FileNameWithPath = String.Empty;
			FileName = String.Empty;
			LastModifiedTime = DateTime.Now;
		}

		public CHeaderFileRecord( string file_path, string project_name, DateTime last_modified_time )
		{
			Project = project_name;
			FileNameWithPath = file_path;
			FileName = Path.GetFileName( file_path );

			LastModifiedTime = last_modified_time;
		}

		// Properties
		[ DataMember( Name="File", Order = 0, IsRequired=true ) ]
		public string FileNameWithPath { get; private set; }

		public string FileName { get; private set; }

		[ DataMember( Name="Project", Order = 1, IsRequired=true ) ]
		public string Project { get; private set; }

		[ DataMember( Name="LastModified", Order = 2, IsRequired=true ) ]
		public DateTime LastModifiedTime { get; private set; }
	}

	[ DataContract( Name="EnumEntry", Namespace="http://www.bretambrose.com" ) ]
	public sealed class CEnumEntry
	{
		// Construction
		public CEnumEntry()
		{
			EntryName = string.Empty;
			Value = 0;
		}

		public CEnumEntry( string entry_name, ulong value )
		{
			EntryName = entry_name;
			Value = value;
		}

		// Methods
		// Public interface
		public bool Value_Equals( CEnumEntry enum_entry )
		{
			return EntryName == enum_entry.EntryName && Value == enum_entry.Value;
		}

		// Properties
		[ DataMember( Name="EntryName", Order = 0, IsRequired=true ) ]
		public string EntryName { get; private set; }

		[ DataMember( Name="Value", Order = 1, IsRequired=true ) ]
		public ulong Value { get; private set; }
	}

	[Flags]
	public enum EEnumFlags
	{
		None = 0,
		IsBitfield = 1
	}

	[ DataContract( Name="Enum", Namespace="http://www.bretambrose.com" ) ]
	public sealed class CEnumRecord
	{
		// Construction
		public CEnumRecord()
		{
			Name = String.Empty;
			FileNameWithPath = String.Empty;
			Flags = EEnumFlags.None;
			EnumEntries = new List< CEnumEntry >();
			HeaderFileID = EHeaderFileID.Invalid;
		}

		public CEnumRecord( string name, string file_name_with_path, EEnumFlags flags )
		{
			Name = name;
			FileNameWithPath = file_name_with_path;
			Flags = flags;
			EnumEntries = new List< CEnumEntry >();
			HeaderFileID = EHeaderFileID.Invalid;
		}

		// Methods
		// Public interface
		public bool Value_Equals( CEnumRecord enum_definition )
		{
			if ( Name != enum_definition.Name )
			{
				return false;
			}

			if ( Flags != enum_definition.Flags )
			{
				return false;
			}

			if ( HeaderFileID != enum_definition.HeaderFileID )
			{
				return false;
			}

			if ( EnumEntries.Count != enum_definition.EnumEntries.Count )
			{
				return false;
			}

			for ( int i = 0; i < EnumEntries.Count; i++ )
			{
				if ( !EnumEntries[ i ].Value_Equals( enum_definition.EnumEntries[ i ] ) )
				{
					return false;
				}
			}

			return true;
		}

		public void Add_Entry( ulong enum_value, string value_name )
		{
			string upper_value_name = value_name.ToUpper();

			if ( EnumEntries.Find( e => upper_value_name == e.EntryName ) != null )
			{
				throw new Exception( "Duplicate enum value name ( " + upper_value_name + " ) in enum " + Name );
			} 

			EnumEntries.Add( new CEnumEntry( upper_value_name, enum_value ) );
		}

		public IEnumerable< CEnumEntry > Get_Entries()
		{
			return EnumEntries;
		}

		// Properties
		[ DataMember( Name="Name", Order = 0, IsRequired=true ) ]
		public string Name { get; private set; }

		[ DataMember( Name="FileNameWithPath", Order = 1, IsRequired=true ) ]
		public string FileNameWithPath { get; private set; }

		[ DataMember( Name="Flags", Order = 2, IsRequired=true ) ]
		public EEnumFlags Flags { get; private set; }

		[ DataMember( Name="Entries", Order = 3, IsRequired=true ) ]
		private List< CEnumEntry > EnumEntries { get; set; }

		public EHeaderFileID HeaderFileID { get; set; }
	}

	[ DataContract( Name="Project", Namespace="http://www.bretambrose.com" ) ]
	public sealed class CProjectRecord
	{
		// Construction
		public CProjectRecord()
		{
			Name = String.Empty;
			CaseName = String.Empty;
		}

		public CProjectRecord( string name )
		{
			CaseName = name;
			Name = name.ToUpper();
		}

		// Properties
		[ DataMember( Name="Name", Order = 0, IsRequired=true ) ]
		public string Name { get; private set; }

		public string CaseName { get; private set; }
	}

	[DataContract(Name="CEnumXMLDatabase",Namespace="http://www.bretambrose.com")]
	public sealed class CEnumXMLDatabase
	{
		// Construction
		static CEnumXMLDatabase() {}

		CEnumXMLDatabase() 
		{ 
			HeaderFiles = new List< CHeaderFileRecord >();
			Enums = new List< CEnumRecord >();
			Projects = new List< CProjectRecord >();
		}

		// Methods
		// Public interface
		static public void Load_Config()
		{
			try
			{
				DataContractSerializer serializer = new DataContractSerializer( typeof( CEnumXMLDatabase ) );
				using ( Stream stream = File.OpenRead( Build_Filename() ) )
				{
					m_Instance = serializer.ReadObject( stream ) as CEnumXMLDatabase;
				}
			}
			catch ( Exception )
			{
				m_Instance = new CEnumXMLDatabase();
			}		
		}
		
		static public void Save_Config()
		{			
			try
			{
				DataContractSerializer serializer = new DataContractSerializer( typeof( CEnumXMLDatabase ) );
				XmlWriterSettings output_settings = new XmlWriterSettings() { Indent = true };
				
				using ( XmlWriter writer = XmlWriter.Create( Build_Filename(), output_settings ) )
				{
					serializer.WriteObject( writer, m_Instance );
				}
			}
			catch ( Exception e )
			{
				Console.WriteLine( "Exception writing config file:" );
				Console.WriteLine( e.StackTrace );
			}
		}

		public void Initialize_From_Trackers( IEnumerable< CProjectRecord > project_records,
														  IEnumerable< CHeaderFileRecord > file_records, 
														  IEnumerable< CEnumRecord > enum_records )
		{
			Projects.Clear();
			project_records.Apply( pr => Projects.Add( pr ) );

			HeaderFiles.Clear();
			file_records.Apply( hfr => HeaderFiles.Add( hfr ) );

			Enums.Clear();
			enum_records.Apply( er => Enums.Add( er ) );
		}
		
		// Private interface
		private static string Build_Filename()
		{
			return FileNamePrefix + ".xml";
		}

		// Properties
		public static CEnumXMLDatabase Instance { get { return m_Instance; } }

		[DataMember(Name="Projects", Order = 0, IsRequired=true)]
		public List< CProjectRecord > Projects { get; private set; }
		
		[DataMember(Name="HeaderFiles", Order = 1, IsRequired=true)]
		public List< CHeaderFileRecord > HeaderFiles { get; private set; }

		[DataMember(Name="Enums", Order = 2, IsRequired=true)]
		public List< CEnumRecord > Enums { get; private set; }

		// Fields
		private static CEnumXMLDatabase m_Instance = new CEnumXMLDatabase();
		
		private const string FileNamePrefix = "Run/Tools/Data/XML/EnumReflectionDB";
						
	}	
}
