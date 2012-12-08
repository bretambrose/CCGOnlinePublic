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
			BoundName = "";
			CPPName = "";
			HasValue = false;
		}

		public CEnumEntry( string cpp_name, ulong value )
		{
			EntryName = "";
			Value = value;
			BoundName = "";
			CPPName = cpp_name;
			HasValue = true;
		}

		public CEnumEntry( string cpp_name, string entry_name, ulong value )
		{
			EntryName = entry_name;
			Value = value;
			BoundName = "";
			CPPName = cpp_name;
			HasValue = true;
		}

		public CEnumEntry( string cpp_name, string entry_name, string bound_name )
		{
			EntryName = entry_name;
			Value = 0;
			BoundName = bound_name;
			CPPName = cpp_name;
			HasValue = false;
		}

		// Methods
		// Public interface
		public bool Value_Equals( CEnumEntry enum_entry )
		{
			return EntryName == enum_entry.EntryName && Value == enum_entry.Value && BoundName == enum_entry.BoundName;
		}

		public CEnumEntry Clone()
		{
			CEnumEntry clone = new CEnumEntry();

			clone.EntryName = EntryName;
			clone.Value = Value;
			clone.BoundName = BoundName;
			clone.CPPName = CPPName;
			clone.HasValue = HasValue;

			return clone;
		}

		public void Bind_Value( ulong value )
		{
			if ( HasValue )
			{
				throw new Exception( "Attempted to bind a value to an enum entry that already has a value" );
			}

			Value = value;
		}

		public void Unbind_Value()
		{
			HasValue = false;
		}

		// Properties
		[ DataMember( Name="EntryName", Order = 0, IsRequired=true ) ]
		public string EntryName { get; private set; }

		[ DataMember( Name="Value", Order = 1, IsRequired=true ) ]
		public ulong Value { get; private set; }

		[ DataMember( Name="BoundName", Order = 2, IsRequired=true ) ]
		public string BoundName { get; private set; }

		[ DataMember( Name="CPPName", Order = 3, IsRequired=true ) ]
		public string CPPName { get; private set; }

		public bool HasValue { get; private set; }
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
			EnumName = String.Empty;
			FullName = String.Empty;
			FileNameWithPath = String.Empty;
			Namespace = String.Empty;
			Flags = EEnumFlags.None;
			ExtendsEnum = String.Empty;
			EnumEntries = new List< CEnumEntry >();
			HeaderFileID = EHeaderFileID.Invalid;
			ExtensionInitialized = false;
			StartingValue = 0;
		}

		public CEnumRecord( string name, string file_name_with_path, string name_space, string extends_enum, EEnumFlags flags )
		{
			EnumName = name;
			FullName = Build_Full_Enum_Name( name, name_space );
			FileNameWithPath = file_name_with_path;
			Namespace = name_space;
			Flags = flags;
			ExtendsEnum = extends_enum;
			EnumEntries = new List< CEnumEntry >();
			HeaderFileID = EHeaderFileID.Invalid;
			ExtensionInitialized = false;
			StartingValue = 0;
		}

		// Methods
		// Public interface
		public bool Value_Equals( CEnumRecord enum_definition )
		{
			if ( EnumName != enum_definition.EnumName )
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

			if ( Namespace != enum_definition.Namespace )
			{
				return false;
			}

			if ( ExtendsEnum != enum_definition.ExtendsEnum )
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

		public CEnumRecord Clone()
		{
			CEnumRecord record = new CEnumRecord();
			record.EnumName = EnumName;
			record.FullName = FullName;
			record.FileNameWithPath = FileNameWithPath;
			record.Namespace = Namespace;
			record.Flags = Flags;
			record.ExtendsEnum = ExtendsEnum;
			record.HeaderFileID = HeaderFileID;
			record.EnumEntries = new List< CEnumEntry >();
			record.StartingValue = StartingValue;
			record.ExtensionInitialized = false;

			foreach ( var entry in EnumEntries )
			{
				record.EnumEntries.Add( entry.Clone() );
			}

			return record;
		}

		public void Add_Bound_Entry( string cpp_name, string value_name, ulong enum_value )
		{
			string upper_value_name = value_name.ToUpper();
			
			if ( value_name.Length > 0 )
			{
				if ( EnumEntries.Find( e => upper_value_name == e.EntryName ) != null )
				{
					throw new Exception( "Duplicate enum value name ( " + upper_value_name + " ) in enum " + FullName );
				} 
			}

			EnumEntries.Add( new CEnumEntry( cpp_name, upper_value_name, enum_value ) );
		}

		public void Add_Unbound_Entry( string cpp_name, string value_name, string bound_name )
		{
			string upper_value_name = value_name.ToUpper();

			if ( value_name.Length > 0 )
			{
				if ( EnumEntries.Find( e => upper_value_name == e.EntryName ) != null )
				{
					throw new Exception( "Duplicate enum value name ( " + upper_value_name + " ) in enum " + FullName );
				}
			}

			EnumEntries.Add( new CEnumEntry( cpp_name, upper_value_name, bound_name ) );
		}

		public void Bind_Unbound_Values()
		{
			ulong current_value = StartingValue;

			foreach( var entry in EnumEntries )
			{
				if ( !entry.HasValue )
				{
					entry.Bind_Value( current_value );

					if ( Is_Bitfield() )
					{
						current_value <<= 1;
					}
					else
					{
						++current_value;
					}
				}
				else
				{
					throw new Exception( "Extension enum " + FullName + " has an illegally bound entry " + entry.EntryName );
				}
			}

			ExtensionInitialized = true;
		}

		public void Unbind_Values()
		{
			foreach( var entry in EnumEntries )
			{
				entry.Unbind_Value();
			}

			ExtensionInitialized = false;
		}

		public IEnumerable< CEnumEntry > Get_Entries()
		{
			return EnumEntries;
		}

		public CEnumEntry Get_Entry_By_CPP_Name( string cpp_name )
		{
			foreach ( var entry in EnumEntries )
			{
				if ( entry.CPPName == cpp_name )
				{
					return entry;
				}
			}

			return null;
		}

		public bool Is_Bitfield()
		{
			return ( Flags & EEnumFlags.IsBitfield ) != 0;
		}

		// private interface
		private string Build_Full_Enum_Name( string enum_name, string namespace_name )
		{
			if ( namespace_name.Length > 0 )
			{
				return namespace_name + "::" + enum_name;
			}
			
			return enum_name;
		}

		[OnDeserialized]
		private void OnDeserialized( StreamingContext context )
		{
			FullName = Build_Full_Enum_Name( EnumName, Namespace );
		}

		// Properties
		[ DataMember( Name="EnumName", Order = 0, IsRequired=true ) ]
		public string EnumName { get; private set; }

		[ DataMember( Name="FileNameWithPath", Order = 1, IsRequired=true ) ]
		public string FileNameWithPath { get; private set; }

		[ DataMember( Name="Flags", Order = 2, IsRequired=true ) ]
		public EEnumFlags Flags { get; private set; }

		[ DataMember( Name="Entries", Order = 3, IsRequired=true ) ]
		private List< CEnumEntry > EnumEntries { get; set; }

		[ DataMember( Name="Namespace", Order = 4, IsRequired=true ) ]
		public string Namespace { get; private set; }

		[ DataMember( Name="ExtendsEnum", Order = 5, IsRequired=true ) ]
		public string ExtendsEnum { get; private set; }

		public string FullName { get; private set; }
		public EHeaderFileID HeaderFileID { get; set; }
		public CEnumRecord BaseEnum { get; set; }
		public ulong StartingValue { get; set; }
		public bool ExtensionInitialized { get; set; }
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
