USE testdb;

-- drop all procedures
BEGIN TRANSACTION

DECLARE @procedure_name_iterator VARCHAR(255);
DECLARE @procedure_schema_name_iterator VARCHAR(255);

DECLARE procedure_cursor CURSOR FOR 
	SELECT 
		p.name,
		s.name
	FROM sys.procedures AS p
		INNER JOIN sys.schemas AS s ON
			p.schema_id = s.schema_id;

OPEN procedure_cursor;
FETCH NEXT FROM procedure_cursor INTO @procedure_name_iterator, @procedure_schema_name_iterator;

WHILE @@FETCH_STATUS = 0
BEGIN
	DECLARE @drop_procedure_command VARCHAR(512);

	SET @drop_procedure_command = 'DROP PROCEDURE ' + @procedure_schema_name_iterator + '.' + @procedure_name_iterator + ';';
	EXEC( @drop_procedure_command );

	FETCH NEXT FROM procedure_cursor INTO @procedure_name_iterator, @procedure_schema_name_iterator;
END

CLOSE procedure_cursor;
DEALLOCATE procedure_cursor;

COMMIT

-- drop all tables

BEGIN TRANSACTION

DECLARE @table_name_iterator VARCHAR(255);
DECLARE @table_schema_name_iterator VARCHAR(255);

DECLARE table_cursor CURSOR FOR 
	SELECT TABLE_NAME, TABLE_SCHEMA
	FROM INFORMATION_SCHEMA.TABLES 
    WHERE TABLE_TYPE = 'BASE TABLE';

OPEN table_cursor;
FETCH NEXT FROM table_cursor INTO @table_name_iterator, @table_schema_name_iterator;

WHILE @@FETCH_STATUS = 0
BEGIN
	DECLARE @drop_table_command VARCHAR(512);

	SET @drop_table_command = 'DROP TABLE ' + @table_schema_name_iterator + '.' + @table_name_iterator + ';';
	EXEC( @drop_table_command );

	FETCH NEXT FROM table_cursor INTO @table_name_iterator, @table_schema_name_iterator;
END

CLOSE table_cursor;
DEALLOCATE table_cursor;

COMMIT
