CREATE TABLE test_table 
  (
    account_id NUMBER PRIMARY KEY,
    account_name VARCHAR2(255)
  ) 
  TABLESPACE CCGTABLES;

CREATE INDEX test_index ON test_table 
  (
    account_name
  ) 
  TABLESPACE CCGTABLES;
  
CREATE FUNCTION GET_TEST_ACCOUNT_NAME( acct_id IN NUMBER )
  return NUMBER
  is
  acct_name VARCHAR2( 255 );
BEGIN
  SELECT
    account_name INTO acct_name
  FROM
    test_table
  WHERE
    account_id = acct_id;
    
  RETURN acct_name;
END;


