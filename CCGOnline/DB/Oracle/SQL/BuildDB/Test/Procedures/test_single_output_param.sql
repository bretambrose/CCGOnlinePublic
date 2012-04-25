CREATE OR REPLACE PROCEDURE test_single_output_param( 
  p_email IN VARCHAR2,
  p_account_id OUT NUMBER
)
IS
BEGIN

  SELECT
    account_id INTO p_account_id
  FROM
    d_accounts
  WHERE
    upper_account_email = UPPER( p_email ) AND closed_on IS NULL;
    
END test_single_output_param;




