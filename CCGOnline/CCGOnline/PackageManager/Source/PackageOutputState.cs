/**********************************************************************************************************************

	PackageOutputState.cs
		Component that tracks per-output state for the package manager and initiates all background operations for a
		given input

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