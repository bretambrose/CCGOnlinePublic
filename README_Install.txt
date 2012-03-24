Additional Software Installation Steps

Python
 1.  Download and install ActivePython 3.2 or later - http://www.activestate.com/activepython/downloads
 2.  Make sure the python install directory is in your path and python.exe is the associated executable for the .py extension.

Perl
 1.  Download and install ActivePerl 5.14.2 or later, from http://www.activestate.com/activeperl/downloads

Database

Why Oracle?  There are 4 reasonable choices for a DB Server: MySQL, SQLServer, PostgreSQL, and Oracle.  I tried and rejected MySQL because it didn't suit
my usage patterns (doesn't compile stored procedures, poor scripting support).  SQLServer's ODBC driver has been buggy for several years now; they've
fixed some, but there are still unavoidable issues with output parameters and return values used via batched store proc calls.  I also have had a lot
of difficulties with install/uninstall and SQL Server.  
I don't know anything about PostgreSQL, but I fear similar critical weaknesses like with MySQL.  Oracle is free to use for development and learning 
and since this isn't a commercial project and Oracle has the best feature set, I figured why not give it a try.

 1.  Download the Oracle11gR2 (11.2.0.1.0) from http://http://www.oracle.com/technetwork/database/enterprise-edition/downloads/index.html.  Unzip to a common directory
 2.  Run setup.exe, choose create an initial database.
 3.  Select all defaults, write down your system password, give full net access to applicable processes during the install
 4.  Copy the file SampleDBSettings.txt in CCGOnline/DB and name the copy "DBSettings.txt".
 5.  In DBSettings.txt, Edit the "SYSTEMPassword=" line to assign the system password you chose in step 3.
 6.  Run the file "one_time_initialize.py" located in CCGOnline/DB/Python.
 7.  (Not yet implemented) Run the file "rebuild_shard_db.py" located in CCGOnline/DB/Python.




