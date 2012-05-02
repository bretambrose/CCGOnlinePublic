USE testdb;
GO

IF EXISTS (SELECT * FROM sys.objects WHERE type = 'P' AND name = 'get_all_accounts_with_in_out')
	DROP PROCEDURE dynamic.get_all_accounts_with_in_out;
GO

CREATE PROCEDURE dynamic.get_all_accounts_with_in_out
	(
		@p_count BIGINT OUTPUT
	)
AS
BEGIN

	SELECT
		@p_count = COUNT( account_id )
	FROM
		dynamic.accounts
	WHERE
		closed_on IS NULL;

	SELECT
		account_id,
		nickname,
		nickname_sequence_id
	FROM
		dynamic.accounts
	WHERE
		closed_on IS NULL;
    
END;

GO


