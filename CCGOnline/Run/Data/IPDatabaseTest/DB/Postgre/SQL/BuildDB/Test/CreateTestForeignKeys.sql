BEGIN TRANSACTION

ALTER TABLE static.game_product_keys
  ADD CONSTRAINT s_game_product_keys_f1 FOREIGN KEY (game_product_id) REFERENCES enum.game_products(game_product_id) ;
 
ALTER TABLE dynamic.account_products
  ADD CONSTRAINT d_account_products_f1 FOREIGN KEY (game_product_key_id) REFERENCES static.game_product_keys(game_product_key_id);
ALTER TABLE dynamic.account_products
  ADD CONSTRAINT d_account_products_f2 FOREIGN KEY (account_id) REFERENCES dynamic.accounts(account_id);

ALTER TABLE dynamic.account_product_state_log
  ADD CONSTRAINT d_account_product_state_log_f1 FOREIGN KEY (account_product_id) REFERENCES dynamic.account_products(account_product_id);
ALTER TABLE dynamic.account_product_state_log
  ADD CONSTRAINT d_account_product_state_log_f2 FOREIGN KEY (status) REFERENCES enum.game_product_statuses(game_product_status_id);

COMMIT

  