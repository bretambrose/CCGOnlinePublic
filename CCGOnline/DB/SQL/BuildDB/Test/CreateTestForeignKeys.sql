ALTER TABLE s_game_product_keys
  ADD CONSTRAINT s_game_product_keys_f1 FOREIGN KEY (game_product_id) REFERENCES e_game_products(game_product_id) ;
 
ALTER TABLE d_account_products
  ADD CONSTRAINT d_account_products_f1 FOREIGN KEY (game_product_key_id) REFERENCES s_game_product_keys(game_product_key_id);
ALTER TABLE d_account_products
  ADD CONSTRAINT d_account_products_f2 FOREIGN KEY (account_id) REFERENCES d_accounts(account_id);

ALTER TABLE d_account_product_state_log
  ADD CONSTRAINT d_account_product_state_log_f1 FOREIGN KEY (account_product_id) REFERENCES d_account_products(account_product_id);

  