/**********************************************************************************************************************

	DownloadWorker.cs
		A background worker that downloads a package from a webserver.

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
using System.Net;
using System.Text;
using System.IO;
using System.ComponentModel;

namespace PackageManager
{
	public class CDownloadWorker : CWorker
	{
		// Construction
		public CDownloadWorker( CPackageInputEntry package )
		{
			m_Package = package;
		}

		// Methods
		// Protected interface
		protected override void OnDoWork( DoWorkEventArgs event_args )
		{
			bool downloaded = false;

			foreach ( var mirror in m_Package.Mirrors )
			{
				if ( ( mirror.OS & CPackageManager.Get_OS() ) == CPackageManager.Get_OS() )
				{
					string target_filename = m_Package.Name + mirror.Extension;

					using ( WebClient wc = new WebClient() )
					{
						try
						{
							DownloadedFileName = CPackageManager.DownloadDirectory + @"/" + target_filename;
							wc.Proxy = null;
							wc.DownloadFile( new Uri( mirror.URL ), DownloadedFileName );
							downloaded = true;
						}
						catch ( Exception e )
						{
							StringBuilder error_msg = new StringBuilder();
							error_msg.Append( "Error downloading package " );
							error_msg.Append( m_Package.Name );
							error_msg.Append( " from URL " );
							error_msg.Append( mirror.URL );
							error_msg.Append( "\n" );
							error_msg.Append( "Exception message: " );
							error_msg.Append( e.Message );

							Console.WriteLine( error_msg.ToString() );
						}
					}

					if ( downloaded )
					{
						break;
					}	
				}	
			}

			if ( !downloaded )
			{
				throw new Exception( "Unable to download package: " + m_Package.Name );
			}
		}

		// Properties
		public string DownloadedFileName { get; private set; }

		// Fields
		CPackageInputEntry m_Package = null;
	}
}