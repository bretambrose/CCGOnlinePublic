/**********************************************************************************************************************

	[Placeholder for eventual source license]

	PackageInputState.cs
		Component that tracks per-input state for the package manager and initiates all background operations for a
		given input

	(c) Copyright 2011, Bret Ambrose.  All rights reserved.

**********************************************************************************************************************/

using System;

namespace PackageManager
{
	public enum EPackageInputID
	{
		Invalid = 0
	}

	public enum EInputPackageState
	{
		Start,
		PendingDownload,
		Downloading,
		HashingDownload,
		Decompressing,
		Finished,

		Error
	}

	public class CPackageInputState
	{
		// Construction
		public CPackageInputState( CPackageInputEntry config )
		{
			ID = ++m_IDTracker;
			Config = config;
			State = EInputPackageState.Start;
			DownloadedFile = "";
		}

		// Methods
		// Public interface
		public void Service()
		{
			switch ( State )
			{	
				case EInputPackageState.Start:
					State = EInputPackageState.PendingDownload;
					break;

				case EInputPackageState.PendingDownload:
					Service_Pending_Download();
					break;

				case EInputPackageState.Downloading:
					Service_Downloading();
					break;

				case EInputPackageState.HashingDownload:
					Service_Hashing_Download();
					break;

				case EInputPackageState.Decompressing:
					Service_Decompressing();
					break;

				default:
					break;
			}
		}

		// Private interface
		private void Service_Pending_Download()
		{
			if ( m_BackgroundTask == null && CPackageManager.Get_Input_Packages_In_State_Count( EInputPackageState.Downloading ) < CConfigSettings.Instance.MaxConcurrentDownloads )
			{
				State = EInputPackageState.Downloading;
				m_BackgroundTask = new CDownloadWorker( Config );
				m_BackgroundTask.RunWorkerAsync();

				Console.WriteLine( "Downloading package: " + Config.Name );
			}
		}

		private void Service_Downloading()
		{
			switch ( m_BackgroundTask.Get_Completion_Status() )
			{
				case EWorkStatus.CompletionFailure:
				case EWorkStatus.Invalid:
					throw new Exception( "Attempt to download package failed" );

				case EWorkStatus.CompletionSuccess:
					Console.WriteLine( "Successfully downloaded package: " + Config.Name );
					DownloadedFile = ( m_BackgroundTask as CDownloadWorker ).DownloadedFileName;
					State = EInputPackageState.HashingDownload;
					m_BackgroundTask = new CHashWorker( DownloadedFile, 8 );
					m_BackgroundTask.RunWorkerAsync();
					Console.WriteLine( "Hashing package: " + Config.Name );
					break;
			}
		}

		private void Service_Hashing_Download()
		{
			switch ( m_BackgroundTask.Get_Completion_Status() )
			{
				case EWorkStatus.CompletionFailure:
				case EWorkStatus.Invalid:
					throw new Exception( "Attempt to hash downloaded file failed" );

				case EWorkStatus.CompletionSuccess:
					Console.WriteLine( "Successfully hashed downloaded file: " + DownloadedFile );
					CHash download_hash = ( m_BackgroundTask as CHashWorker ).Hash;
					if ( !Config.Verify_Download_Hash( download_hash ) )
					{
						throw new Exception( "Download hash check failure on package: " + Config.Name );
					}
					State = EInputPackageState.Decompressing;
					m_BackgroundTask = new CDecompressWorker( DownloadedFile, CPackageManager.UnpackDirectory );
					m_BackgroundTask.RunWorkerAsync();
					Console.WriteLine( "Decompressing package: " + Config.Name );
					break;
			}
		}

		private void Service_Decompressing()
		{
			switch ( m_BackgroundTask.Get_Completion_Status() )
			{
				case EWorkStatus.CompletionFailure:
				case EWorkStatus.Invalid:
					throw new Exception( "Attempt to decompress file failed" );

				case EWorkStatus.CompletionSuccess:
					Console.WriteLine( "Successfully decompressed downloaded file: " + DownloadedFile );
					State = EInputPackageState.Finished;
					m_BackgroundTask = null;
					break;
			}
		}

		// Properties
		public EPackageInputID ID { get; private set; }
		public CPackageInputEntry Config { get; private set; }
		public EInputPackageState State { get; set; }
		private string DownloadedFile { get; set; }

		// Fields
		private static EPackageInputID m_IDTracker = EPackageInputID.Invalid;

		private CWorker m_BackgroundTask = null;

	}
}