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
	[DataContract(Name="CFileEntry",Namespace="http://www.bretambrose.com")]
	public sealed class CFileEntry
	{
		public CFileEntry()
		{
			Project = String.Empty;
			FileNameWithPath = String.Empty;
			FileName = String.Empty;
			LastModifiedTime = DateTime.Now;
		}

		public CFileEntry( string file_path, DateTime last_modified_time )
		{
			int separator_index = file_path.IndexOf( Path.PathSeparator );
			Project = file_path.Substring( 0, separator_index );
			FileNameWithPath = file_path;
			FileName = Path.GetFileName( file_path );

			LastModifiedTime = last_modified_time;
		}

		public string Project { get; private set; }
		public string FileNameWithPath { get; private set; }
		public string FileName { get; private set; }
		public DateTime LastModifiedTime { get; private set; }
	}

	[Flags]
	public enum EEnumFlags
	{
		None = 0,
		IsBitfield = 1
	}

	[DataContract(Name="CEnumEntry",Namespace="http://www.bretambrose.com")]
	public sealed class CEnumEntry
	{
		public CEnumEntry()
		{
			Name = String.Empty;
			Flags = EEnumFlags.None;
			EnumEntries = new Dictionary< string, ulong >();
		}

		public CEnumEntry( string name, EEnumFlags flags )
		{
			Name = name;
			Flags = flags;
			EnumEntries = new Dictionary< string, ulong >();
		}

		public void Add_Entry( ulong enum_value, string value_name )
		{
			string upper_value_name = value_name.ToUpper();

			if ( EnumEntries.ContainsKey( upper_value_name ) )
			{
				throw new Exception( "Duplicate enum value name ( " + upper_value_name + " ) in enum " + Name );
			} 

			EnumEntries.Add( upper_value_name, enum_value );
		}

		public IEnumerable< KeyValuePair< string, ulong > > Get_Entries()
		{
			return EnumEntries;
		}

		public string Name { get; private set; }
		public EEnumFlags Flags { get; private set; }
		private Dictionary< string, ulong > EnumEntries { get; set; }
	}

	[DataContract(Name="CEnumXMLDatabase",Namespace="http://www.bretambrose.com")]
	public sealed class CEnumXMLDatabase
	{
		// Construction
		static CEnumXMLDatabase() {}

		CEnumXMLDatabase() 
		{ 
			Files = new List< CFileEntry >();
			Enums = new List< CEnumEntry >();
		}

		// Methods
		// Public interface
		static public void Make_Sample_Config()
		{
			m_Instance.Files.Clear();
			m_Instance.Enums.Clear();

			m_Instance.Files.Add( new CFileEntry( "TestProject\\SomeFile.h", DateTime.Now ) );
			m_Instance.Files.Add( new CFileEntry( "TestProject\\SomeDir\\AnotherFile.h", DateTime.Now ) );
			m_Instance.Files.Add( new CFileEntry( "AnotherProject\\AnotherFile.h", DateTime.Now ) );

			CEnumEntry enum_entry = new CEnumEntry( "ETestEnum", EEnumFlags.None );
			enum_entry.Add_Entry( 0, "Invalid" );
			enum_entry.Add_Entry( 1, "Test1" );
			enum_entry.Add_Entry( 2, "Test2" );

			m_Instance.Enums.Add( enum_entry );
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

		public void Initialize( IEnumerable< CFileEntry > file_entries, IEnumerable< CEnumEntry > enum_entries )
		{
			Files.Clear();
			file_entries.Apply( fe => Files.Add( fe ) );

			Enums.Clear();
			enum_entries.Apply( ee => Enums.Add( ee ) );
		}
		
		// Properties
		public static CEnumXMLDatabase Instance { get { return m_Instance; } }
		
		[DataMember(Name="Files", Order = 1, IsRequired=true)]
		public List< CFileEntry > Files { get; private set; }

		[DataMember(Name="Enums", Order = 2, IsRequired=true)]
		public List< CEnumEntry > Enums { get; private set; }

		// Fields
		private static CEnumXMLDatabase m_Instance = new CEnumXMLDatabase();
		
		private const string FileName = "Run/Tools/Data/XML/EnumReflectionDB.xml";
						
	}	
}
