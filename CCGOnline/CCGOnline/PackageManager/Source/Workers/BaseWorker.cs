﻿/**********************************************************************************************************************

	[Placeholder for eventual source license]

	BaseWorker.cs
		Base class for all background workers, contains some state tracking utility functions and locking mechanism
		for safe cross-thread access.

	(c) Copyright 2011, Bret Ambrose.  All rights reserved.

**********************************************************************************************************************/

using System;
using System.ComponentModel;

namespace PackageManager
{
	public enum EWorkStatus
	{
		Invalid,

		InProgress,
		CompletionSuccess,
		CompletionFailure
	}

	// FFS why do I have to include this attribute to keep the designer from activating on double click?
	[System.ComponentModel.DesignerCategory("")]
	public class CWorker : BackgroundWorker
	{
		// Construction
		public CWorker()
		{
		}

		// Methods
		// Public interface
		public EWorkStatus Get_Completion_Status()
		{
			EWorkStatus status = EWorkStatus.Invalid;
			// possibly safe to read from this without a lock, but just to be safe I'll do so anyway
			lock ( m_Lock )
			{
				status = m_CompletionStatus;
			}

			return status;
		}

		public string Get_Error_Message()
		{
			string message = null;
			// possibly safe to read from this without a lock, but just to be safe I'll do so anyway
			lock ( m_Lock )
			{
				message = m_ErrorMessage;
			}

			return message;
		}

		// Protected interface
		protected override void OnRunWorkerCompleted( RunWorkerCompletedEventArgs event_args )
		{
			if ( event_args.Error != null || event_args.Cancelled )
			{
				Set_Completion_Status( EWorkStatus.CompletionFailure, event_args.Error.Message );
			}
			else
			{
				Set_Completion_Status( EWorkStatus.CompletionSuccess, null );
			}
		}

		// private interface
		public void Set_Completion_Status( EWorkStatus status, string error_message )
		{
			lock( m_Lock )
			{
				m_CompletionStatus = status;
				m_ErrorMessage = error_message;
			}
		}

		// Fields
		private object m_Lock = new object();
		private EWorkStatus m_CompletionStatus = EWorkStatus.InProgress;
		private string m_ErrorMessage = null;
	}
}