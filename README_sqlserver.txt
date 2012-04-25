(1) Download SQL Server 2012 64 bit from http://www.microsoft.com/download/en/details.aspx?id=29066
(2) Install SQL Server 2012, Choose Express Edition
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
