CCGOnlinePublic Building And Installation

You need Visual Studio 2013 to build this project.  It's advised to apply sp1 as well (and possibly a requirement of installing SQL Server 2012).


One-time installation Steps:

1. Fork and pull the project to a local git repository

2. Load and build the release configuration of the CLRTools solution.

3. Run the batch file SyncPackages_Clean.bat, found in CCGOnlinePublic/CCGOnline/Run/Tools.  This step downloads and installs external dependencies that 
are too large/bulky for a GitHub project.  Depending on the system, this step can take several minutes.

4. Run the batch file CopyDlls.bat, found in CCGOnlinePublic/CCGOnline/Run/Tools.

5. Execute IPPreBuild.vsix, which can be found in CCGOnlinePublic/CCGOnline/CCGOnline/IPPreBuild/bin/Release.  This installs an extension which will do 
codegeneration for the project before it gets built.

Once you've completed these steps you'll be able to open the main solution (CCGOnline.sln) and build.  These steps only need to be performed once.


Running executables in the project:

Executables are copied into the appropriate (Client/Server/Test/Tools) directory beneath CCGOnline/Run.  You'll need to point Visual Studio at the appropriate
executable.  You'll also need to set the working directory to where the executable is located.  Finally, you'll need to update the PATH environment variable
to include the appropriate External_DLL_XX directory underneath the working directory.  So in the Debugging settings, set Environment to

PATH=%PATH%;External_DLL_XX

where XX is 32 or 64 depending on what configuration you're setting up.