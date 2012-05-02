-- CREATE DATABASE blah;

--USE MASTER;
--CREATE LOGIN blahuser WITH PASSWORD = 'blah', DEFAULT_DATABASE = blah;

-- DROP LOGIN blahuser;

--USE blah;

--CREATE USER blahuser FOR LOGIN blahuser;

-- DROP LOGIN blahuser;

--DROP DATABASE blah;

/*
DECLARE @login_name_iterator VARCHAR(255);

DECLARE login_cursor CURSOR FOR 
	SELECT loginname
	FROM master.dbo.syslogins 
    WHERE dbname = 'testdb' OR dbname = 'ccgdb';

OPEN login_cursor;
FETCH NEXT FROM login_cursor INTO @login_name_iterator;

WHILE @@FETCH_STATUS = 0
BEGIN
	DROP LOGIN @login_name_iterator;

	FETCH NEXT FROM login_cursor INTO @login_name_iterator;
END

CLOSE login_cursor;
DEALLOCATE login_cursor;
*/