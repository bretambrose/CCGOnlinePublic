CREATE DATABASE testdb ON ( 
	NAME = test_db_data,
    FILENAME = 'C:\Program Files\Microsoft SQL Server\MSSQL11.CCGONLINE\MSSQL\DATA\test_db_data.mdf',
    SIZE = 50MB,
    MAXSIZE = 100MB,
    FILEGROWTH = 10MB )
GO

CREATE DATABASE ccgdb ON ( 
	NAME = ccg_db_data,
    FILENAME = 'C:\Program Files\Microsoft SQL Server\MSSQL11.CCGONLINE\MSSQL\DATA\ccg_db_data.mdf',
    SIZE = 100MB,
    MAXSIZE = 1GB,
    FILEGROWTH = 50MB )
GO
