CREATE OR REPLACE PROCEDURE get_account_by_email( 
  p_email IN VARCHAR2,
  p_account_cursor OUT SYS_REFCURSOR
)
IS
BEGIN

  OPEN p_account_cursor FOR
  SELECT
    account_id,
    nickname,
    nickname_sequence_id
  FROM
    d_accounts
  WHERE
    upper_account_email = UPPER( p_email ) AND closed_on IS NULL;
    
END get_account_by_email;




