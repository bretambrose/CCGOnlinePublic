/**********************************************************************************************************************

	[Placeholder for eventual source license]

	OutputManifest.cs
		Singleton component that tracks all output hashes, reading/writing them from/to xml as necessary

	(c) Copyright 2011, Bret Ambrose.  All rights reserved.

**********************************************************************************************************************/

using System;
using System.Collections.Generic;
using System.Text;
using System.Runtime.Serialization;
using System.Xml.Serialization;
using System.Xml;
using System.IO;
	
namespace PackageManager
{	

	[DataContract(Name="OutputEntry", Namespace="http://www.bretambrose.com")]
	public class COutputManifestEntry
	{
		public COutputManifestEntry() 
		{
			OutputTag = "";
			Clear_Hash();
		}

		public COutputManifestEntry( string output_tag, CHash hash )
		{
			OutputTag = output_tag;
			Clear_Hash();
			Hash = hash;
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

		[DataMember(Name="OutputTag", Order = 0, IsRequired=true)]
		public string OutputTag { get; private set; }

		[DataMember(Name="Hash", Order = 1, IsRequired=false)]
		private byte[] RawHash { get; set; }
		public CHash Hash { get; set; }
	}

	[DataContract(Name="COutputManifest",Namespace="http://www.bretambrose.com")]
	public sealed class COutputManifest
	{
		// Construction
		static COutputManifest() {}
		COutputManifest() 
		{ 
			Outputs = new List< COutputManifestEntry >();
		}

		// Methods
		// Public interface
		static public void Load()
		{
			try
			{
				DataContractSerializer serializer = new DataContractSerializer( typeof( COutputManifest ) );
				using ( Stream stream = File.OpenRead( FileName ) )
				{
					m_Instance = serializer.ReadObject( stream ) as COutputManifest;
				}
			}
			catch ( FileNotFoundException )
			{
				m_Instance = new COutputManifest(); // intentionally swallow
			}			
		}
		
		static public void Save()
		{		
			try
			{
				DataContractSerializer serializer = new DataContractSerializer( typeof( COutputManifest ) );
				XmlWriterSettings output_settings = new XmlWriterSettings() { Indent = true };
				
				using ( XmlWriter writer = XmlWriter.Create( FileName, output_settings ) )
				{
					serializer.WriteObject( writer, m_Instance );
				}
			}
			catch ( Exception )
			{
				;
			}
		}

		public static void Delete()
		{
			File.Delete( FileName );
		}
		
		// Properties
		public static COutputManifest Instance { get { return m_Instance; } }

		[DataMember(Name="Outputs", Order = 1, IsRequired=true)]
		public List< COutputManifestEntry > Outputs { get; private set; }

		// Fields
		private static COutputManifest m_Instance = new COutputManifest();
		
		private const string FileName = "Run/Tools/Data/XML/OutputManifest.xml";
						
	}	

}