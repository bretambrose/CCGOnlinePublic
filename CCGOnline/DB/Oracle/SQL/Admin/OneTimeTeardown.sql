DECLARE
  user_count INTEGER;
  table_space_count INTEGER;
BEGIN

  -- TEST cleanup
  -- Drop server account
  SELECT
    COUNT(*) into user_count
  FROM
    dba_users
  WHERE
    USERNAME = 'TESTSERVER';
    
  IF user_count = 1 THEN
    EXECUTE IMMEDIATE 'DROP USER TESTSERVER CASCADE';
  END IF; 

  -- Drop dev account  
  SELECT
    COUNT(*) into user_count
  FROM
    dba_users
  WHERE
    USERNAME = 'TEST';
    
  IF user_count = 1 THEN
    EXECUTE IMMEDIATE 'DROP USER TEST CASCADE';
  END IF; 

  -- Drop test indexspace
  SELECT 
    COUNT(*) into table_space_count
  FROM 
    dba_tablespaces 
  WHERE 
    tablespace_name = 'TESTINDICES';
    
  IF table_space_count = 1 THEN
    EXECUTE IMMEDIATE 'DROP TABLESPACE TESTINDICES INCLUDING CONTENTS AND DATAFILES';
  END IF;
  
  -- Drop test tablespace
  SELECT 
    COUNT(*) into table_space_count
  FROM 
    dba_tablespaces 
  WHERE 
    tablespace_name = 'TESTTABLES';
    
  IF table_space_count = 1 THEN
    EXECUTE IMMEDIATE 'DROP TABLESPACE TESTTABLES INCLUDING CONTENTS AND DATAFILES';
  END IF;
  
  -------------------------------------------------------------------------------------------
  -- CCG cleanup
  -- Drop report account
  SELECT
    COUNT(*) into user_count
  FROM
    dba_users
  WHERE
    USERNAME = 'CCGREPORT';
    
  IF user_count = 1 THEN
    EXECUTE IMMEDIATE 'DROP USER CCGREPORT CASCADE';
  END IF; 

  -- Drop server account
  SELECT
    COUNT(*) into user_count
  FROM
    dba_users
  WHERE
    USERNAME = 'CCGSERVER';
    
  IF user_count = 1 THEN
    EXECUTE IMMEDIATE 'DROP USER CCGSERVER CASCADE';
  END IF; 

  -- Drop dev account  
  SELECT
    COUNT(*) into user_count
  FROM
    dba_users
  WHERE
    USERNAME = 'CCG';
    
  IF user_count = 1 THEN
    EXECUTE IMMEDIATE 'DROP USER CCG CASCADE';
  END IF; 

  -- Drop ccg indexspace
  SELECT 
    COUNT(*) into table_space_count
  FROM 
    dba_tablespaces 
  WHERE 
    tablespace_name = 'CCGINDICES';
    
  IF table_space_count = 1 THEN
    EXECUTE IMMEDIATE 'DROP TABLESPACE CCGINDICES INCLUDING CONTENTS AND DATAFILES';
  END IF;
  
  -- Drop ccg tablespace
  SELECT 
    COUNT(*) into table_space_count
  FROM 
    dba_tablespaces 
  WHERE 
    tablespace_name = 'CCGTABLES';
    
  IF table_space_count = 1 THEN
    EXECUTE IMMEDIATE 'DROP TABLESPACE CCGTABLES INCLUDING CONTENTS AND DATAFILES';
  END IF;

  EXECUTE IMMEDIATE 'PURGE RECYCLEBIN';
  
  COMMIT;
END;



