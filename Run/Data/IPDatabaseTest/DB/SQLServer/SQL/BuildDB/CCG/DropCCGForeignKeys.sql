DECLARE
  fk_name VARCHAR2(30);
  table_name VARCHAR2(30);
  CURSOR fk_cursor IS SELECT CONSTRAINT_NAME, TABLE_NAME FROM user_constraints WHERE OWNER = 'CCG' AND CONSTRAINT_TYPE = 'R';
BEGIN
  -- drop all foreign keys
  OPEN fk_cursor;

  LOOP
    FETCH fk_cursor INTO fk_name, table_name;
    EXIT WHEN fk_cursor%NOTFOUND;
    EXECUTE IMMEDIATE ( 'ALTER TABLE ' || table_name || ' DROP CONSTRAINT ' || fk_name );
  END LOOP;
  
  CLOSE fk_cursor;
  
  EXECUTE IMMEDIATE 'PURGE RECYCLEBIN';
END;