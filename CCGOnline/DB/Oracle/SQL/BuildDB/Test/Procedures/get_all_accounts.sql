CREATE OR REPLACE PROCEDURE get_all_accounts( 
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
   closed_on IS NULL;
    
END get_all_accounts;



