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
		public CHeaderFileRecord()
		{
			Project = String.Empty;
			FileNameWithPath = String.Empty;
			FileName = String.Empty;
			LastModifiedTime = DateTime.Now;
		}

		public CHeaderFileRecord( string file_path, DateTime last_modified_time )
		{
			int separator_index = file_path.IndexOf( Path.PathSeparator );
			Project = file_path.Substring( 0, separator_index );
			FileNameWithPath = file_path;
			FileName = Path.GetFileName( file_path );

			LastModifiedTime = last_modified_time;
		}

		[ DataMember( Name="File", Order = 0, IsRequired=true ) ]
		public string FileNameWithPath { get; private set; }

		public string FileName { get; private set; }
		public string Project { get; private set; }

		[ DataMember( Name="LastModified", Order = 1, IsRequired=true ) ]
		public DateTime LastModifiedTime { get; private set; }
	}

	[ DataContract( Name="EnumEntry", Namespace="http://www.bretambrose.com" ) ]
	public sealed class CEnumEntry
	{
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

		public bool Value_Equals( CEnumEntry enum_entry )
		{
			return EntryName == enum_entry.EntryName && Value == enum_entry.Value;
		}

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
		public CEnumRecord()
		{
			Name = String.Empty;
			HeaderFileName = String.Empty;
			Flags = EEnumFlags.None;
			EnumEntries = new List< CEnumEntry >();
			HeaderFileID = EHeaderFileID.Invalid;
		}

		public CEnumRecord( string name, string header_file_name, EEnumFlags flags )
		{
			Name = name;
			HeaderFileName = header_file_name;
			Flags = flags;
			EnumEntries = new List< CEnumEntry >();
			HeaderFileID = EHeaderFileID.Invalid;
		}

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

		[ DataMember( Name="Name", Order = 0, IsRequired=true ) ]
		public string Name { get; private set; }

		[ DataMember( Name="HeaderFile", Order = 1, IsRequired=true ) ]
		public string HeaderFileName { get; private set; }

		[ DataMember( Name="Flags", Order = 2, IsRequired=true ) ]
		public EEnumFlags Flags { get; private set; }

		[ DataMember( Name="Entries", Order = 3, IsRequired=true ) ]
		private List< CEnumEntry > EnumEntries { get; set; }

		public EHeaderFileID HeaderFileID { get; set; }
	}

	[ DataContract( Name="Project", Namespace="http://www.bretambrose.com" ) ]
	public sealed class CProjectRecord
	{
		public CProjectRecord()
		{
			Name = String.Empty;
		}

		public CProjectRecord( string name )
		{
			Name = name.ToUpper();
		}

		[ DataMember( Name="Name", Order = 0, IsRequired=true ) ]
		public string Name { get; private set; }
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
		static public void Make_Sample_Config()
		{
			m_Instance.Projects.Clear();
			m_Instance.HeaderFiles.Clear();
			m_Instance.Enums.Clear();

			m_Instance.Projects.Add( new CProjectRecord( "TestProject" ) );
			m_Instance.Projects.Add( new CProjectRecord( "AnotherProject" ) );

			m_Instance.HeaderFiles.Add( new CHeaderFileRecord( "TestProject\\SomeFile.h", DateTime.Now ) );
			m_Instance.HeaderFiles.Add( new CHeaderFileRecord( "TestProject\\SomeDir\\AnotherFile.h", DateTime.Now ) );
			m_Instance.HeaderFiles.Add( new CHeaderFileRecord( "AnotherProject\\AnotherFile.h", DateTime.Now ) );

			CEnumRecord enum_record = new CEnumRecord( "ETestEnum", "TestProject\\SomeFile.h", EEnumFlags.None );
			enum_record.Add_Entry( 0, "Invalid" );
			enum_record.Add_Entry( 1, "Test1" );
			enum_record.Add_Entry( 2, "Test2" );

			m_Instance.Enums.Add( enum_record );
		}

		static public void Load_Config()
		{
			try
			{
				DataContractSerializer serializer = new DataContractSerializer( typeof( CEnumXMLDatabase ) );
				using ( Stream stream = File.OpenRead( FileName ) )
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
				
				using ( XmlWriter writer = XmlWriter.Create( FileName, output_settings ) )
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
		
		private const string FileName = "Run/Tools/Data/XML/EnumReflectionDB.xml";
						
	}	
}
