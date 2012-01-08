CCGOnlinePublic Building And Installation

You need Visual Studio 2010 to build this project.

One-time installation Steps:

1. Fork and pull the project to a local git repository

2. Run the batch file SyncPackages_Clean.bat, found in CCGOnlinePublic/CCGOnline/Run/Tools.  This step downloads and installs external dependencies that are too large/bulky for a GitHub project.  Depending on the system, this step can take 15+ minutes (boost is culprit here).

3. Open the Visual Studio Macro IDE.  Add the OnBuildBegin handler found in CCGOnlinePublic/CCGOnline/CCGOnline/Macros/EnvironmentEvents.vb into your local EnvironmentEvents definition.  Make sure you don't add the handler into the auto-generated section.

Once you've completed these three steps you'll be able to open the main solution (CCGOnline.sln) and build.  These steps only need to be performed once.