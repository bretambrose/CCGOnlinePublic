CREATE OR REPLACE PROCEDURE add_account( 
  p_email IN VARCHAR2,
  p_nickname IN VARCHAR2,
  p_password_hash IN VARCHAR2
)
IS
  upper_email VARCHAR2(255);
  upper_nick VARCHAR2(255);
  acct_id NUMBER(19);
  nickname_seq_id NUMBER(10);
  now TIMESTAMP;
BEGIN
  upper_email := UPPER( p_email );
  upper_nick := UPPER( p_nickname );
  acct_id := d_accounts_s1.NextVal;
  
  SELECT
    COUNT( * ) + 1 INTO nickname_seq_id
  FROM
    d_accounts
  WHERE
    upper_nickname = upper_nick;
  
  SELECT
    CURRENT_TIMESTAMP INTO now
  FROM
    dual;
    
  INSERT INTO d_accounts
  VALUES
    ( acct_id, p_email, upper_email, p_password_hash, p_nickname, upper_nick, nickname_seq_id, now, NULL );
  
END add_account;



