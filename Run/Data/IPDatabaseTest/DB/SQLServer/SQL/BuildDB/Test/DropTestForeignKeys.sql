USE testdb;

BEGIN TRANSACTION

DECLARE @fk_name_iterator VARCHAR(255);
DECLARE @table_name_iterator VARCHAR(255);
DECLARE @schema_name_iterator VARCHAR(255);

DECLARE fk_cursor CURSOR FOR 
	SELECT CONSTRAINT_NAME, TABLE_NAME, TABLE_SCHEMA
	FROM INFORMATION_SCHEMA.TABLE_CONSTRAINTS
    WHERE CONSTRAINT_TYPE = 'FOREIGN KEY';

OPEN fk_cursor;
FETCH NEXT FROM fk_cursor INTO @fk_name_iterator, @table_name_iterator, @schema_name_iterator;

WHILE @@FETCH_STATUS = 0
BEGIN
	DECLARE @drop_fk_command VARCHAR(512);

	SET @drop_fk_command = 'ALTER TABLE ' + @schema_name_iterator + '.' + @table_name_iterator + ' DROP CONSTRAINT ' + @fk_name_iterator + ';';
	EXEC( @drop_fk_command );

	FETCH NEXT FROM fk_cursor INTO @fk_name_iterator, @table_name_iterator, @schema_name_iterator;
END

CLOSE fk_cursor;
DEALLOCATE fk_cursor;

COMMIT
