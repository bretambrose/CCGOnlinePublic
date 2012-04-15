DECLARE
  acct_id NUMBER(19);
  product_key_id NUMBER(19);
  acct_product_id NUMBER(19);
  now TIMESTAMP;
BEGIN

  SELECT systimestamp AT TIME ZONE 'GMT' INTO now FROM dual;

  INSERT INTO e_account_statuses
    VALUES ( 1, 'ACTIVE', 0 );
  INSERT INTO e_account_statuses
    VALUES ( 2, 'LOCKED', 0 );
  INSERT INTO e_account_statuses
    VALUES ( 3, 'CLOSED', 0 );    
    
  INSERT INTO e_game_products
    VALUES ( 1, 'NETRUNNER', 0 );
  INSERT INTO e_game_products
    VALUES ( 2, 'JYHAD', 0 );
  INSERT INTO e_game_products
    VALUES ( 3, 'LEGENDOFTHEFIVERINGS', 0 );    
    
  INSERT INTO e_game_product_statuses
    VALUES ( 1, 'ACTIVE', 0 );
  INSERT INTO e_game_product_statuses
    VALUES ( 2, 'EXPIRED', 0 );
  INSERT INTO e_game_product_statuses
    VALUES ( 3, 'BANNED', 0 );

  INSERT INTO s_game_product_keys
    VALUES ( s_game_product_keys_s1.NextVal, sys_guid(), 1 );   
  INSERT INTO s_game_product_keys
    VALUES ( s_game_product_keys_s1.NextVal, sys_guid(), 1 );
  INSERT INTO s_game_product_keys
    VALUES ( s_game_product_keys_s1.NextVal, sys_guid(), 1 );
  INSERT INTO s_game_product_keys
    VALUES ( s_game_product_keys_s1.NextVal, sys_guid(), 1 );

  INSERT INTO s_game_product_keys
    VALUES ( s_game_product_keys_s1.NextVal, sys_guid(), 2 );
  INSERT INTO s_game_product_keys
    VALUES ( s_game_product_keys_s1.NextVal, sys_guid(), 2 );
  INSERT INTO s_game_product_keys
    VALUES ( s_game_product_keys_s1.NextVal, sys_guid(), 2 );
  INSERT INTO s_game_product_keys
    VALUES ( s_game_product_keys_s1.NextVal, sys_guid(), 2 );
    
  INSERT INTO s_game_product_keys
    VALUES ( s_game_product_keys_s1.NextVal, sys_guid(), 3 );
  INSERT INTO s_game_product_keys
    VALUES ( s_game_product_keys_s1.NextVal, sys_guid(), 3 );
  INSERT INTO s_game_product_keys
    VALUES ( s_game_product_keys_s1.NextVal, sys_guid(), 3 );
  INSERT INTO s_game_product_keys
    VALUES ( s_game_product_keys_s1.NextVal, sys_guid(), 3 );   

  INSERT INTO d_accounts
    VALUES ( d_accounts_s1.NextVal, 
             'bretambrose@gmail.com', 
             'BRETAMBROSE@GMAIL.COM',
             0,
             'Bret',
             'BRET',
             1,
             now,
             NULL );
             
  INSERT INTO d_accounts
    VALUES ( d_accounts_s1.NextVal, 
             'petra222@yahoo.com', 
             'PETRA222@YAHOO.COM',
             0,
             'Peti',
             'PETI',
             1,
             now,
             NULL );   
             
  INSERT INTO d_accounts
    VALUES ( d_accounts_s1.NextVal, 
             'will@mailinator.com', 
             'WILL@MAILINATOR.COM',
             0,
             'Will',
             'WILL',
             1,
             now,
             NULL );    
 
  -- netrunner registration for Bret         
  SELECT account_id INTO acct_id FROM d_accounts WHERE nickname = 'Bret';

  SELECT game_product_key_id INTO product_key_id FROM s_game_product_keys WHERE game_product_id = 1 AND rownum = 1;
  
  INSERT INTO d_account_products
    VALUES ( d_account_products_s1.NextVal, acct_id, product_key_id, now );
  
  SELECT account_product_id INTO acct_product_id FROM d_account_products WHERE account_id = acct_id AND game_product_key_id = product_key_id;
  
  INSERT INTO d_account_product_state_log
    VALUES ( d_account_product_state_log_s1.NextVal, acct_product_id, 1, 'Initial Registration', now, NULL ); 
  
  -- jyhad registration for Bret     
  SELECT game_product_key_id INTO product_key_id FROM s_game_product_keys WHERE game_product_id = 2 AND rownum = 1;
  
  INSERT INTO d_account_products
    VALUES ( d_account_products_s1.NextVal, acct_id, product_key_id, now );
  
  SELECT account_product_id INTO acct_product_id FROM d_account_products WHERE account_id = acct_id AND game_product_key_id = product_key_id;
  
  INSERT INTO d_account_product_state_log
    VALUES ( d_account_product_state_log_s1.NextVal, acct_product_id, 1, 'Initial Registration', now, NULL );

  -- L5R registration for Bret        
  SELECT game_product_key_id INTO product_key_id FROM s_game_product_keys WHERE game_product_id = 3 AND rownum = 1;
  
  INSERT INTO d_account_products
    VALUES ( d_account_products_s1.NextVal, acct_id, product_key_id, now );
  
  SELECT account_product_id INTO acct_product_id FROM d_account_products WHERE account_id = acct_id AND game_product_key_id = product_key_id;
  
  INSERT INTO d_account_product_state_log
    VALUES ( d_account_product_state_log_s1.NextVal, acct_product_id, 1, 'Initial Registration', now, NULL );
    
  COMMIT;
END;


