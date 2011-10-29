/**********************************************************************************************************************

	[Placeholder for eventual source license]

	PackageOutputState.cs
		Component that tracks per-output state for the package manager and initiates all background operations for a
		given input

	(c) Copyright 2011, Bret Ambrose.  All rights reserved.

**********************************************************************************************************************/

using System;

namespace PackageManager
{
	public enum EPackageOutputID
	{
		Invalid = 0
	}

	public enum EOutputPackageState
	{
		WaitingOnInput,
		CopyAndHash,
		Finished,

		Error
	}

	public class CPackageOutputState
	{
		// Construction
		public CPackageOutputState( CPackageOutputEntry config, EPackageInputID input_id )
		{
			ID = ++m_IDTracker;
			InputID = input_id;
			Config = config;
			State = EOutputPackageState.WaitingOnInput;
		}

		// Methods
		// Public interface
		public void Service()
		{
			switch ( State )
			{
				case EOutputPackageState.WaitingOnInput:
					Service_Waiting_On_Input();
					break;

				case EOutputPackageState.CopyAndHash:
					Service_Copy_And_Hash();
					break;

			}
		}

		// Private interface
		private void Service_Waiting_On_Input()
		{
			CPackageInputState input = CPackageManager.Get_Package_Input( InputID );
			if ( input.State == EInputPackageState.Finished )
			{
				State = EOutputPackageState.CopyAndHash;
				m_BackgroundTask = new CCopyAndHashWorker( CPackageManager.UnpackDirectory + @"/" + Config.Source, Config.Destination );
				m_BackgroundTask.RunWorkerAsync();
				Console.WriteLine( "Building output: " + Config.Tag );
			}
		}

		private void Service_Copy_And_Hash()
		{
			switch ( m_BackgroundTask.Get_Completion_Status() )
			{
				case EWorkStatus.CompletionFailure:
				case EWorkStatus.Invalid:
					throw new Exception( "Attempt to copy and hash package failed" );

				case EWorkStatus.CompletionSuccess:
					Console.WriteLine( "Successfully built output: " + Config.Tag );
					Update_Output_Hashes( ( m_BackgroundTask as CCopyAndHashWorker ).CumulativeHash );

					State = EOutputPackageState.Finished;
					m_BackgroundTask = null;
					break;
			}
		}

		private void Update_Output_Hashes( CHash hash )
		{
			bool modified_existing = false;
			foreach ( var manifest_entry in COutputManifest.Instance.Outputs )
			{
				if ( manifest_entry.OutputTag == Config.Tag )
				{
					manifest_entry.Hash = hash;
					modified_existing = true;
					break;
				}
			}

			if ( !modified_existing )
			{
				COutputManifest.Instance.Outputs.Add( new COutputManifestEntry( Config.Tag, hash ) );
			}

			Config.Hash = hash;
		}

		// Properties
		public EPackageOutputID ID { get; private set; }
		public EPackageInputID InputID { get; private set; }
		public CPackageOutputEntry Config { get; private set; }
		public EOutputPackageState State { get; set; }

		// Fields
		static EPackageOutputID m_IDTracker = EPackageOutputID.Invalid;

		private CWorker m_BackgroundTask = null;
	}
}