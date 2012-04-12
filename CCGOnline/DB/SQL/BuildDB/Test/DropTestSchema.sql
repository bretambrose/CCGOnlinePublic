DECLARE
  table_name VARCHAR2(30);
  procedure_name VARCHAR2(64);
  function_name VARCHAR2(64);
  sequence_name VARCHAR2(64);
  CURSOR table_cursor IS SELECT TABLE_NAME FROM user_tables; 
  CURSOR procedure_cursor IS SELECT OBJECT_NAME FROM user_procedures WHERE object_type = 'PROCEDURE';
  CURSOR function_cursor IS SELECT OBJECT_NAME FROM user_procedures WHERE object_type = 'FUNCTION';
  CURSOR sequence_cursor IS SELECT SEQUENCE_NAME FROM user_sequences;   
BEGIN
  -- drop all procedures
  OPEN procedure_cursor;

  LOOP
    FETCH procedure_cursor INTO procedure_name;
    EXIT WHEN procedure_cursor%NOTFOUND;
    EXECUTE IMMEDIATE ( 'DROP PROCEDURE ' || procedure_name );
  END LOOP;
  
  CLOSE procedure_cursor;

  -- drop all functions  
  OPEN function_cursor;

  LOOP
    FETCH function_cursor INTO function_name;
    EXIT WHEN function_cursor%NOTFOUND;
    EXECUTE IMMEDIATE ( 'DROP FUNCTION ' || function_name );
  END LOOP;
  
  CLOSE function_cursor;

  -- drop all tables
  OPEN table_cursor;

  LOOP
    FETCH table_cursor INTO table_name;
    EXIT WHEN table_cursor%NOTFOUND;
    EXECUTE IMMEDIATE ( 'DROP TABLE ' || table_name );
  END LOOP;
  
  CLOSE table_cursor;

  -- drop all sequences
  OPEN sequence_cursor;

  LOOP
    FETCH sequence_cursor INTO sequence_name;
    EXIT WHEN sequence_cursor%NOTFOUND;
    EXECUTE IMMEDIATE ( 'DROP SEQUENCE ' || sequence_name );
  END LOOP;
  
  CLOSE sequence_cursor;
  
  EXECUTE IMMEDIATE 'PURGE RECYCLEBIN';
END;

/
