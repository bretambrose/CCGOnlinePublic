USE testdb;
GO

IF EXISTS (SELECT * FROM sys.objects WHERE type = 'TF' AND name = 'tabled_valued_function')
	DROP FUNCTION dynamic.tabled_valued_function;
GO

CREATE FUNCTION dynamic.tabled_valued_function
	(
		@p_account_id BIGINT
	)
RETURNS @test_table TABLE
	(
		product_desc VARCHAR(255),
		product_key_desc VARCHAR(36)
	)
AS
BEGIN

	INSERT INTO @test_table
		SELECT 
			gp.game_product_desc,
			gpk.game_product_key_desc
		FROM
			dynamic.account_products AS ap
			INNER JOIN dynamic.account_product_state_log AS apsl ON
				ap.account_product_id = apsl.account_product_id AND status_end IS NULL AND apsl.status = 1
			INNER JOIN static.game_product_keys AS gpk ON
				ap.game_product_key_id = gpk.game_product_key_id
			INNER JOIN enum.game_products AS gp ON
				gpk.game_product_id = gp.game_product_id
		WHERE
			account_id = @p_account_id;
    
	RETURN;
END;

GO

