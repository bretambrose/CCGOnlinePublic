/**********************************************************************************************************************

	OutputManifest.cs
		Singleton component that tracks all output hashes, reading/writing them from/to xml as necessary

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