Additional Software Installation Steps

Python
 1.  Download and install ActivePython 3.2 or later - http://www.activestate.com/activepython/downloads
 2.  Make sure the python install directory is in your path and python.exe is the associated executable for the .py extension (this is not
	the default of the installation and needs to be manually added during the install).

Database

There are 4 reasonable choices for a DB Server: MySQL, SQLServer, PostgreSQL, and Oracle.  I tried and rejected MySQL because it didn't suit
my usage patterns (doesn't compile stored procedures, poor scripting support).  The ODBC driver included with Oracle 11Gr2 crashed badly on simple
batch operations.  And Oracle is a really evil corporation.  I don't know anything about PostgreSQL, but I fear similar critical weaknesses like with 
MySQL.  SQLServer's ODBC driver has been buggy for several years now but at least it doesn't crash on dead simple stuff, so SQL Server it is.  

(1) Download SQL Server 2012 64 bit from http://www.microsoft.com/download/en/details.aspx?id=29066
(2) Install SQL Server 2012, Choose Express Edition
(2.5) Select all features
(3) Use CCGONLINE for the instance name
(4) At Database Engine Configuration:
	A) Choose Mixed Mode Authentication
	B) Select a password for the 'sa' admin account.  Write this down.
(5) Reboot your computer
(6) Copy SampleDBSettings.txt in the DB/SQLServer folder to DBSettings.txt
(7) In DBSettings.txt, set DB to the full SQLServer instance name, most likely something like [your pc's name]\CCGONLINE
(8) In DBSettings.txt, set AdminPassword to what you chose in Step 4B.
(9) In the DB/SQLServer/Python directory, run one_time_setup.py
(10) In the DB/SQLServer/Python directory, run rebuild_test_db.py and rebuild_ccg_db.py

Note that the [your pc's name] part of the connection string in the C++ development code is currently hard-coded (not read from the db settings file).
This will change eventually.




