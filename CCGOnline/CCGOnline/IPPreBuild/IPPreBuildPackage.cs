using System;
using System.Diagnostics;
using System.Globalization;
using System.Runtime.InteropServices;
using System.ComponentModel.Design;
using Microsoft.Win32;
using Microsoft.VisualStudio;
using Microsoft.VisualStudio.Shell.Interop;
using Microsoft.VisualStudio.OLE.Interop;
using Microsoft.VisualStudio.Shell;

using EnvDTE;
using EnvDTE80;
using System.IO;
using System.Windows.Forms;

namespace InterminableProcesses.IPPreBuild
{
	/// <summary>
	/// This is the class that implements the package exposed by this assembly.
	///
	/// The minimum requirement for a class to be considered a valid package for Visual Studio
	/// is to implement the IVsPackage interface and register itself with the shell.
	/// This package uses the helper classes defined inside the Managed Package Framework (MPF)
	/// to do it: it derives from the Package class that provides the implementation of the 
	/// IVsPackage interface and uses the registration attributes defined in the framework to 
	/// register itself and its components with the shell.
	/// </summary>
	// This attribute tells the PkgDef creation utility (CreatePkgDef.exe) that this class is
	// a package.
	[PackageRegistration(UseManagedResourcesOnly = true)]
	// This attribute is used to register the information needed to show this package
	// in the Help/About dialog of Visual Studio.
	[InstalledProductRegistration("#110", "#112", "1.0", IconResourceID = 400)]
	[ProvideAutoLoad("{f1536ef8-92ec-443c-9ed7-fdadf150da82}")]
	[Guid(GuidList.guidIPPreBuildPkgString)]
	public sealed class IPPreBuildPackage : Package
	{
		/// <summary>
		/// Default constructor of the package.
		/// Inside this method you can place any initialization code that does not require 
		/// any Visual Studio service because at this point the package object is created but 
		/// not sited yet inside Visual Studio environment. The place to do all the other 
		/// initialization is the Initialize method.
		/// </summary>
		public IPPreBuildPackage()
		{
		}

		/////////////////////////////////////////////////////////////////////////////
		// Overridden Package Implementation
		#region Package Members

		/// <summary>
		/// Initialization of the package; this method is called right after the package is sited, so this is the place
		/// where you can put all the initialization code that rely on services provided by VisualStudio.
		/// </summary>
		protected override void Initialize()
		{
			base.Initialize();

			m_dte = (DTE2)GetGlobalService(typeof(EnvDTE.DTE));

			m_buildEvents = m_dte.Events.BuildEvents;

			m_buildEvents.OnBuildBegin += new _dispBuildEvents_OnBuildBeginEventHandler(OnBuildBegin);
		}
		
		#endregion

		private void InvokeCodeGeneration()
		{
			if ( m_dte.Solution.FullName.EndsWith( "CCGOnline.sln" ) )
			{
				Directory.SetCurrentDirectory(Path.GetDirectoryName(m_dte.Solution.FullName));
				Directory.SetCurrentDirectory("../Run/Tools");

				ProcessStartInfo psi = new System.Diagnostics.ProcessStartInfo("EnumReflector.exe");
				psi.RedirectStandardOutput = false;
				psi.WindowStyle = ProcessWindowStyle.Normal;
				psi.UseShellExecute = true;
				psi.Arguments = "NORMAL CCGOnline";

				System.Diagnostics.Process reflect_process = System.Diagnostics.Process.Start(psi);
				reflect_process.WaitForExit();

				if ( reflect_process.ExitCode == 1 )
				{
					MessageBox.Show("PreBuild: EnumReflector encountered an error, see Log file for details.  Build cancelled.");
					m_dte.ExecuteCommand("Build.Cancel");
				}
			}
		}

		private void CleanupCodeGeneration()
		{
		}

		private void OnBuildBegin(vsBuildScope scope, vsBuildAction action)
		{
			switch(action)
			{ 
				case vsBuildAction.vsBuildActionBuild:
				case vsBuildAction.vsBuildActionRebuildAll:
					InvokeCodeGeneration();
					break;

				case vsBuildAction.vsBuildActionClean:
					CleanupCodeGeneration();
					break;

				default:
					break;
			}
		}

		DTE2 m_dte = null;
		BuildEvents m_buildEvents = null;
	}
}
