USE testdb;

BEGIN TRANSACTION

	DECLARE @now DATETIME2 = SYSUTCDATETIME();

	INSERT INTO enum.account_statuses VALUES 
		( 1, 'ACTIVE', 0 ),
		( 2, 'LOCKED', 0 ),
		( 3, 'CLOSED', 0 );    
   
	INSERT INTO enum.game_products VALUES
		( 1, 'NETRUNNER', 0 ),
		( 2, 'JYHAD', 0 ),
		( 3, 'LEGENDOFTHEFIVERINGS', 0 );    
    
	INSERT INTO enum.game_product_statuses VALUES 
		( 1, 'ACTIVE', 0 ),
		( 2, 'EXPIRED', 0 ),
		( 3, 'BANNED', 0 );

	INSERT INTO static.game_product_keys
		(
			game_product_key_desc,
			game_product_id
		)
	VALUES
		( CAST (NEWID() AS VARCHAR(36)), 1 ),
		( CAST (NEWID() AS VARCHAR(36)), 1 ),
		( CAST (NEWID() AS VARCHAR(36)), 1 ),
		( CAST (NEWID() AS VARCHAR(36)), 1 ),
		( CAST (NEWID() AS VARCHAR(36)), 2 ),
		( CAST (NEWID() AS VARCHAR(36)), 2 ),
		( CAST (NEWID() AS VARCHAR(36)), 2 ),
		( CAST (NEWID() AS VARCHAR(36)), 2 ),
		( CAST (NEWID() AS VARCHAR(36)), 3 ),
		( CAST (NEWID() AS VARCHAR(36)), 3 ),
		( CAST (NEWID() AS VARCHAR(36)), 3 ),
		( CAST (NEWID() AS VARCHAR(36)), 3 );

	INSERT INTO dynamic.accounts
		(
			account_email,
			upper_account_email,
			password_hash,
			nickname,
			upper_nickname,
			nickname_sequence_id,
			created_on,
			closed_on
		)
	VALUES
		( 'bretambrose@gmail.com', 'BRETAMBROSE@GMAIL.COM', '00000000000000000000000000000000', 'Bret', 'BRET', 1, @now, NULL ),
		( 'petra222@yahoo.com', 'PETRA222@YAHOO.COM', '00000000000000000000000000000000', 'Peti', 'PETI', 1, @now, NULL ),
		( 'will@mailinator.com', 'WILL@MAILINATOR.COM', '00000000000000000000000000000000', 'Will', 'WILL', 1, @now, NULL );

	-- product registration for Bret
	DECLARE @account_id BIGINT;
	SELECT @account_id = account_id FROM dynamic.accounts WHERE nickname = 'Bret';

	-- register netrunner
	DECLARE @product_key_id BIGINT;
	SELECT TOP(1) @product_key_id = game_product_key_id FROM static.game_product_keys WHERE game_product_id = 1;

	INSERT INTO dynamic.account_products
		(
			account_id,
			game_product_key_id,
			added_on
		)
    VALUES 
		( @account_id, @product_key_id, @now );

	DECLARE @account_product_id BIGINT;
	SELECT @account_product_id = account_product_id FROM dynamic.account_products WHERE account_id = @account_id AND game_product_key_id = @product_key_id;
  
	INSERT INTO dynamic.account_product_state_log
		(
			account_product_id,
			status,
			status_change_reason,
			status_begin,
			status_end
		)
	VALUES 
		( @account_product_id, 1, 'Initial Registration', @now, NULL ); 

	-- register jyhad
	SELECT TOP(1) @product_key_id = game_product_key_id FROM static.game_product_keys WHERE game_product_id = 2;

	INSERT INTO dynamic.account_products
		(
			account_id,
			game_product_key_id,
			added_on
		)
    VALUES 
		( @account_id, @product_key_id, @now );

	SELECT @account_product_id = account_product_id FROM dynamic.account_products WHERE account_id = @account_id AND game_product_key_id = @product_key_id;
  
	INSERT INTO dynamic.account_product_state_log
		(
			account_product_id,
			status,
			status_change_reason,
			status_begin,
			status_end
		)
	VALUES 
		( @account_product_id, 1, 'Initial Registration', @now, NULL ); 

	-- register L5R
	SELECT TOP(1) @product_key_id = game_product_key_id FROM static.game_product_keys WHERE game_product_id = 3;

	INSERT INTO dynamic.account_products
		(
			account_id,
			game_product_key_id,
			added_on
		)
    VALUES 
		( @account_id, @product_key_id, @now );

	SELECT @account_product_id = account_product_id FROM dynamic.account_products WHERE account_id = @account_id AND game_product_key_id = @product_key_id;
  
	INSERT INTO dynamic.account_product_state_log
		(
			account_product_id,
			status,
			status_change_reason,
			status_begin,
			status_end
		)
	VALUES 
		( @account_product_id, 1, 'Initial Registration', @now, NULL ); 

COMMIT



