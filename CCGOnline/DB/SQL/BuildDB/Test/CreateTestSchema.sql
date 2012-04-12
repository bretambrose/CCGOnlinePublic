CREATE TABLE e_account_statuses
  (
    account_status_id NUMBER(10),
    account_status_desc VARCHAR2(255) NOT NULL,
    flags NUMBER(10) DEFAULT 0 NOT NULL,   
    CONSTRAINT e_account_statuses_pk PRIMARY KEY (account_status_id) USING INDEX TABLESPACE TESTINDICES,
    CONSTRAINT e_account_statuses_u1 UNIQUE (account_status_desc) USING INDEX TABLESPACE TESTINDICES
  ) 
  TABLESPACE TESTTABLES;
  
CREATE TABLE e_game_products
  (
    game_product_id NUMBER(10),
    game_product_desc VARCHAR2(255) NOT NULL,
    flags NUMBER(10) DEFAULT 0 NOT NULL,
    CONSTRAINT e_game_products_pk PRIMARY KEY (game_product_id) USING INDEX TABLESPACE TESTINDICES,
    CONSTRAINT e_game_products_u1 UNIQUE (game_product_desc) USING INDEX TABLESPACE TESTINDICES
  ) 
  TABLESPACE TESTTABLES;

CREATE TABLE e_game_product_statuses
  (
    game_product_status_id NUMBER(10),
    game_product_status_desc VARCHAR2(255) NOT NULL,
    flags NUMBER(10) DEFAULT 0 NOT NULL,    
    CONSTRAINT e_game_product_statuses_pk PRIMARY KEY (game_product_status_id) USING INDEX TABLESPACE TESTINDICES,
    CONSTRAINT e_game_product_statuses_u1 UNIQUE (game_product_status_desc) USING INDEX TABLESPACE TESTINDICES    
  ) 
  TABLESPACE TESTTABLES;
  
CREATE TABLE d_accounts 
  (
    account_id NUMBER(19),
    account_email VARCHAR2(255) NOT NULL,
    upper_account_email VARCHAR2(255) NOT NULL,
    password_hash INTEGER NOT NULL,
    nickname VARCHAR2(32) NOT NULL,
    upper_nickname VARCHAR2(32) NOT NULL,
    nickname_sequence_id NUMBER(10) NOT NULL,
    created_on TIMESTAMP NOT NULL,
    closed_on TIMESTAMP,    
    CONSTRAINT d_accounts_pk PRIMARY KEY (account_id) USING INDEX TABLESPACE TESTINDICES,
    CONSTRAINT d_accounts_u1 UNIQUE (upper_account_email) USING INDEX TABLESPACE TESTINDICES,
    CONSTRAINT d_accounts_u2 UNIQUE (upper_nickname, nickname_sequence_id) USING INDEX TABLESPACE TESTINDICES 
  ) 
  TABLESPACE TESTTABLES;

CREATE SEQUENCE d_accounts_s1 INCREMENT BY 1 START WITH 1;

CREATE TABLE s_game_product_keys
  (
    game_product_key_id NUMBER(19),
    game_product_key_desc VARCHAR2(32) NOT NULL,
    game_product_id NUMBER(10) NOT NULL,   
    CONSTRAINT s_game_product_keys_pk PRIMARY KEY (game_product_key_id) USING INDEX TABLESPACE TESTINDICES,
    CONSTRAINT s_game_product_keys_u1 UNIQUE (game_product_key_desc) USING INDEX TABLESPACE TESTINDICES
  ) 
  TABLESPACE TESTTABLES;

CREATE SEQUENCE s_game_product_keys_s1 INCREMENT BY 1 START WITH 1;

CREATE TABLE d_account_products
  (
    account_product_id NUMBER(19),
    account_id NUMBER(19) NOT NULL,
    game_product_key_id NUMBER(19) NOT NULL,
    added_on TIMESTAMP NOT NULL,    
    CONSTRAINT d_account_products_pk PRIMARY KEY (account_product_id) USING INDEX TABLESPACE TESTINDICES,
    CONSTRAINT d_account_products_u1 UNIQUE (game_product_key_id) USING INDEX TABLESPACE TESTINDICES
  ) 
  TABLESPACE TESTTABLES;  

CREATE SEQUENCE d_account_products_s1 INCREMENT BY 1 START WITH 1;

-- foreign keys: account_product_id
CREATE TABLE d_account_product_state_log
  (
    account_product_state_log_id NUMBER, 
    account_product_id NUMBER(19) NOT NULL,
    status NUMBER(10) NOT NULL,
    status_change_reason VARCHAR2(1024) NOT NULL,
    status_begin TIMESTAMP NOT NULL,
    status_end TIMESTAMP,   
    CONSTRAINT d_account_product_state_log_pk PRIMARY KEY (account_product_state_log_id) USING INDEX TABLESPACE TESTINDICES
  ) 
  TABLESPACE TESTTABLES; 

CREATE SEQUENCE d_account_product_state_log_s1 INCREMENT BY 1 START WITH 1;

