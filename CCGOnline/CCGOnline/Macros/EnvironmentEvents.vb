Option Strict Off
Option Explicit Off
Imports System
Imports EnvDTE
Imports EnvDTE80
Imports EnvDTE90
Imports EnvDTE90a
Imports EnvDTE100
Imports System.Diagnostics
Imports System.Windows.Forms
Imports System.IO

Public Module EnvironmentEvents

    Private Sub BuildEvents_OnBuildBegin(ByVal Scope As EnvDTE.vsBuildScope, ByVal Action As EnvDTE.vsBuildAction) Handles BuildEvents.OnBuildBegin

        Select Case Action

            Case vsBuildAction.vsBuildActionBuild, vsBuildAction.vsBuildActionRebuildAll

                If DTE.Solution.FullName.EndsWith("CCGOnline.sln") Then

                    Directory.SetCurrentDirectory(Path.GetDirectoryName(DTE.Solution.FullName))
                    Directory.SetCurrentDirectory("../Run/Tools")

                    Dim psi As New System.Diagnostics.ProcessStartInfo("EnumReflector.exe")
                    psi.RedirectStandardOutput = False
                    psi.WindowStyle = ProcessWindowStyle.Normal
                    psi.UseShellExecute = True
                    psi.Arguments = "NORMAL CCGOnline"

                    Dim reflectProcess As System.Diagnostics.Process
                    reflectProcess = System.Diagnostics.Process.Start(psi)
                    reflectProcess.WaitForExit()

                    If reflectProcess.ExitCode = 1 Then
                        MessageBox.Show("PreBuild: EnumReflector encountered an error, see Log file for details.  Build cancelled.")
                        DTE.ExecuteCommand("Build.Cancel")
                    End If

                End If

        End Select

    End Sub


#Region "Automatically generated code, do not modify"
    'Automatically generated code, do not modify
    'Event Sources Begin
	<System.ContextStaticAttribute()> Public WithEvents DTEEvents As EnvDTE.DTEEvents
	<System.ContextStaticAttribute()> Public WithEvents DocumentEvents As EnvDTE.DocumentEvents
	<System.ContextStaticAttribute()> Public WithEvents WindowEvents As EnvDTE.WindowEvents
	<System.ContextStaticAttribute()> Public WithEvents TaskListEvents As EnvDTE.TaskListEvents
	<System.ContextStaticAttribute()> Public WithEvents FindEvents As EnvDTE.FindEvents
	<System.ContextStaticAttribute()> Public WithEvents OutputWindowEvents As EnvDTE.OutputWindowEvents
	<System.ContextStaticAttribute()> Public WithEvents SelectionEvents As EnvDTE.SelectionEvents
	<System.ContextStaticAttribute()> Public WithEvents BuildEvents As EnvDTE.BuildEvents
	<System.ContextStaticAttribute()> Public WithEvents SolutionEvents As EnvDTE.SolutionEvents
	<System.ContextStaticAttribute()> Public WithEvents SolutionItemsEvents As EnvDTE.ProjectItemsEvents
	<System.ContextStaticAttribute()> Public WithEvents MiscFilesEvents As EnvDTE.ProjectItemsEvents
	<System.ContextStaticAttribute()> Public WithEvents DebuggerEvents As EnvDTE.DebuggerEvents
	<System.ContextStaticAttribute()> Public WithEvents ProjectsEvents As EnvDTE.ProjectsEvents
	<System.ContextStaticAttribute()> Public WithEvents TextDocumentKeyPressEvents As EnvDTE80.TextDocumentKeyPressEvents
	<System.ContextStaticAttribute()> Public WithEvents CodeModelEvents As EnvDTE80.CodeModelEvents
	<System.ContextStaticAttribute()> Public WithEvents DebuggerProcessEvents As EnvDTE80.DebuggerProcessEvents
	<System.ContextStaticAttribute()> Public WithEvents DebuggerExpressionEvaluationEvents As EnvDTE80.DebuggerExpressionEvaluationEvents
    'Event Sources End
    'End of automatically generated code
#End Region




End Module

