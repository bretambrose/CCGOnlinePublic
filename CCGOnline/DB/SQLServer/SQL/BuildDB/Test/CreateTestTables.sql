USE testdb;

CREATE TABLE enum.account_statuses
	(
		account_status_id BIGINT,
		account_status_desc VARCHAR(255) NOT NULL,
		flags BIGINT DEFAULT 0 NOT NULL,   

		CONSTRAINT e_account_statuses_pk PRIMARY KEY (account_status_id),
		CONSTRAINT e_account_statuses_u1 UNIQUE (account_status_desc)
	);

CREATE TABLE enum.game_products
	(
		game_product_id BIGINT,
		game_product_desc VARCHAR(255) NOT NULL,
		flags BIGINT DEFAULT 0 NOT NULL,

		CONSTRAINT e_game_products_pk PRIMARY KEY (game_product_id),
		CONSTRAINT e_game_products_u1 UNIQUE (game_product_desc)
	);

CREATE TABLE enum.game_product_statuses
	(
		game_product_status_id BIGINT,
		game_product_status_desc VARCHAR(255) NOT NULL,
		flags BIGINT DEFAULT 0 NOT NULL,    

		CONSTRAINT e_game_product_statuses_pk PRIMARY KEY (game_product_status_id),
		CONSTRAINT e_game_product_statuses_u1 UNIQUE (game_product_status_desc)
	);

CREATE TABLE static.game_product_keys
	(
		game_product_key_id BIGINT IDENTITY(1, 1),
		game_product_key_desc VARCHAR(36) NOT NULL,
		game_product_id BIGINT NOT NULL,   

		CONSTRAINT s_game_product_keys_pk PRIMARY KEY (game_product_key_id),
		CONSTRAINT s_game_product_keys_u1 UNIQUE (game_product_key_desc)
	);
    
CREATE TABLE dynamic.accounts 
	(
		account_id BIGINT IDENTITY(1, 1),
		account_email VARCHAR(255) NOT NULL,
		upper_account_email VARCHAR(255) NOT NULL,
		password_hash VARCHAR(32) NOT NULL,
		nickname NVARCHAR(32) NOT NULL,
		upper_nickname NVARCHAR(32) NOT NULL,
		nickname_sequence_id INTEGER NOT NULL,
		created_on DATETIME2 NOT NULL,
		closed_on DATETIME2,
	    
		CONSTRAINT d_accounts_pk PRIMARY KEY (account_id),
		CONSTRAINT d_accounts_u1 UNIQUE (upper_account_email),
		CONSTRAINT d_accounts_u2 UNIQUE (upper_nickname, nickname_sequence_id)
	);

CREATE TABLE dynamic.account_products
	(
		account_product_id BIGINT IDENTITY(1, 1),
		account_id BIGINT NOT NULL,
		game_product_key_id BIGINT NOT NULL,
		added_on DATETIME2 NOT NULL,    

		CONSTRAINT d_account_products_pk PRIMARY KEY (account_product_id),
		CONSTRAINT d_account_products_u1 UNIQUE (game_product_key_id)
	);  

CREATE TABLE dynamic.account_product_state_log
	(
		account_product_state_log_id BIGINT IDENTITY(1, 1), 
		account_product_id BIGINT NOT NULL,
		status BIGINT NOT NULL,
		status_change_reason VARCHAR(1024) NOT NULL,
		status_begin DATETIME2 NOT NULL,
		status_end DATETIME2,   

		CONSTRAINT d_account_product_state_log_pk PRIMARY KEY (account_product_state_log_id)
	); 

CREATE TABLE dynamic.test_transactions
	(
		test_transaction_id BIGINT NOT NULL,
		user_data BIGINT NOT NULL,

		CONSTRAINT d_test_transactions_pk PRIMARY KEY (test_transaction_id)
	);