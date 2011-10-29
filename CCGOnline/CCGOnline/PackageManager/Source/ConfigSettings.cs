/**********************************************************************************************************************

	[Placeholder for eventual source license]

	ConfigSettings.cs
		Singleton wrapper for the PackageManagerConfig.xml file.  The set of inputs and outputs are built from the
		data specification contained within.

	(c) Copyright 2011, Bret Ambrose.  All rights reserved.

**********************************************************************************************************************/

using System;
using System.Collections.Generic;
using System.Text;
using System.Runtime.Serialization;
using System.Xml.Serialization;
using System.Xml;
using System.IO;
using System.Net;
	
namespace PackageManager
{	

	[Flags]
	public enum EOperatingSystem
	{
		Invalid,

		[EnumMember]
		Windows = 1,

		[EnumMember]
		Linux = 2
	}

	[DataContract(Name="PackageLocation", Namespace="http://www.bretambrose.com")]
	public class CPackageLocation
	{
		public CPackageLocation() 
		{
			URL = "";
			OS = EOperatingSystem.Invalid;
			Extension = "";
		}

		public CPackageLocation( string url, EOperatingSystem os )
		{
			URL = url;
			OS = os;
			Calculate_Extension();
		}

		[OnDeserialized]
		private void On_Post_Load( StreamingContext context )
		{
			Calculate_Extension();
		}

		private void Calculate_Extension()
		{
			int extension_position = URL.LastIndexOf( '.' );
			if ( extension_position == -1 )
			{
				Extension = "";
			}

			Extension = URL.Substring( extension_position );
		}

		[DataMember(Name="OS", Order = 0, IsRequired=true)]
		public EOperatingSystem OS { get; private set; }

		[DataMember(Name="URL", Order = 1, IsRequired=true)]
		public string URL { get; private set; }

		public string Extension { get; set; }
	}

	[DataContract(Name="InputPackage", Namespace="http://www.bretambrose.com")]
	public class CPackageInputEntry
	{
		public CPackageInputEntry() 
		{
			Name = "";
			Mirrors = new List< CPackageLocation >();
			Clear_Hash();
		}

		public CPackageInputEntry( string name )
		{
			Name = name;
			Mirrors = new List< CPackageLocation >();
			Clear_Hash();
		}

		public void Clear_Hash()
		{
			RawHash = null;
			Hash = new CHash();
		}

		public bool Verify_Download_Hash( CHash hash )
		{
			if ( !Hash.Is_Valid() )
			{
				Hash = hash;
				return true;
			}

			return Hash.Equals( hash );
		}

		[OnDeserialized]
		private void On_Post_Load( StreamingContext context )
		{
			Hash = new CHash( RawHash );
		}
		
		[OnSerializing]
		private void On_Pre_Save( StreamingContext context )
		{
			RawHash = Hash.Get_Raw_Bytes();
		}

		[DataMember(Name="Name", Order = 0, IsRequired=true)]
		public string Name { get; private set; }

		[DataMember(Name="Hash", Order = 1, IsRequired=false)]
		private byte[] RawHash { get; set; }
		public CHash Hash { get; private set; }

		[DataMember(Name="Mirrors", Order = 2, IsRequired=true)]
		public List< CPackageLocation > Mirrors { get; private set; }
	}

	[DataContract(Name="OutputEntry", Namespace="http://www.bretambrose.com")]
	public class CPackageOutputEntry
	{
		public CPackageOutputEntry() 
		{
			Tag = "";
			PackageName = "";
			Source = "";
			Destination = "";
			Clear_Hash();
		}

		public CPackageOutputEntry( string tag, string package_name, string source, string destination )
		{
			Tag = tag;
			PackageName = package_name;
			Source = source;
			Destination = destination;
			Clear_Hash();
		}

		public void Clear_Hash()
		{
			RawHash = null;
			Hash = new CHash();
		}

		[OnDeserialized]
		private void On_Post_Load( StreamingContext context )
		{
			Hash = new CHash( RawHash );
		}
		
		[OnSerializing]
		private void On_Pre_Save( StreamingContext context )
		{
			RawHash = Hash.Get_Raw_Bytes();
		}

		[DataMember(Name="Tag", Order = 0, IsRequired=true)]
		public string Tag { get; private set; }

		[DataMember(Name="PackageName", Order = 1, IsRequired=true)]
		public string PackageName { get; private set; }

		[DataMember(Name="Source", Order = 2, IsRequired=true)]
		public string Source { get; private set; }

		[DataMember(Name="Destination", Order = 3, IsRequired=true)]
		public string Destination { get; private set; }

		[DataMember(Name="Hash", Order = 4, IsRequired=false)]
		public byte[] RawHash { get; set; }
		public CHash Hash { get; set; }
	}

	[DataContract(Name="CConfigSettings",Namespace="http://www.bretambrose.com")]
	public sealed class CConfigSettings
	{
		// Construction
		static CConfigSettings() {}
		CConfigSettings() 
		{ 
			MaxConcurrentDownloads = 3;
			Inputs = new List< CPackageInputEntry >();
			Outputs = new List< CPackageOutputEntry >();
		}

		// Methods
		// Public interface
		static public void Make_Sample_Config()
		{
			CPackageInputEntry package1 = new CPackageInputEntry( "TestPackage" );

			CPackageLocation location = new CPackageLocation( "ftp://www.testurl.com/files", EOperatingSystem.Windows );
			package1.Mirrors.Add( location );

			location = new CPackageLocation( "ftp://www.testy.com/something/files", EOperatingSystem.Linux );
			package1.Mirrors.Add( location );

			m_Instance.Inputs.Add( package1 );

			CPackageOutputEntry output1 = new CPackageOutputEntry( "Tag1", "TestPackage", "src/", "./TestPackage/" );
			m_Instance.Outputs.Add( output1 );

			CPackageOutputEntry output2 = new CPackageOutputEntry( "Tag2", "TestPackage", "lib/test_package32.dll", "./External_DLL_32/" );
			m_Instance.Outputs.Add( output2 );
		}

		static public void Load_Config()
		{
			DataContractSerializer serializer = new DataContractSerializer( typeof( CConfigSettings ) );
			using ( Stream stream = File.OpenRead( FileName ) )
			{
				m_Instance = serializer.ReadObject( stream ) as CConfigSettings;
			}		
		}
		
		static public void Save_Config()
		{			
			try
			{
				DataContractSerializer serializer = new DataContractSerializer( typeof( CConfigSettings ) );
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
		
		public void Discard_Hashes()
		{
			Inputs.Apply( p => p.Clear_Hash() );
			Outputs.Apply( p => p.Clear_Hash() );
		}
		
		// Properties
		public static CConfigSettings Instance { get { return m_Instance; } }

		[DataMember(Name="MaxConcurrentDownloads", Order = 0, IsRequired=true)]
		public uint MaxConcurrentDownloads { get; private set; }
		
		[DataMember(Name="Inputs", Order = 1, IsRequired=true)]
		public List< CPackageInputEntry > Inputs { get; private set; }

		[DataMember(Name="Outputs", Order = 2, IsRequired=true)]
		public List< CPackageOutputEntry > Outputs { get; private set; }

		// Fields
		private static CConfigSettings m_Instance = new CConfigSettings();
		
		private const string FileName = "Run/Tools/Data/XML/PackageManagerConfig.xml";
						
	}	

}