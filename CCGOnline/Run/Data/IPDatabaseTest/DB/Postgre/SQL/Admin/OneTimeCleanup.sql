-- schema no need
-- users no need

-- logins
IF EXISTS ( SELECT loginname FROM master.dbo.syslogins WHERE loginname = 'test' )
BEGIN
	DROP LOGIN test;
END

IF EXISTS ( SELECT loginname FROM master.dbo.syslogins WHERE loginname = 'testserver' )
BEGIN
	DROP LOGIN testserver;
END

GO

IF EXISTS ( SELECT loginname FROM master.dbo.syslogins WHERE loginname = 'ccg' )
BEGIN
	DROP LOGIN ccg;
END

GO

IF EXISTS ( SELECT loginname FROM master.dbo.syslogins WHERE loginname = 'ccgserver' )
BEGIN
	DROP LOGIN ccgserver;
END

GO

-- databases

IF EXISTS ( SELECT name FROM master.dbo.sysdatabases WHERE name = 'testdb' )
BEGIN
	DROP DATABASE testdb;
END

GO

IF EXISTS ( SELECT name FROM master.dbo.sysdatabases WHERE name = 'ccgdb' )
BEGIN
	DROP DATABASE ccgdb;
END

GO

