USE testdb;
GO

IF EXISTS (SELECT * FROM sys.objects WHERE type = 'P' AND name = 'get_all_accounts_with_in_out')
	DROP PROCEDURE dynamic.get_all_accounts_with_in_out;
GO

CREATE PROCEDURE dynamic.get_all_accounts_with_in_out
	(
		@p_filtered_nickname VARCHAR(32),
		@p_in_test BIGINT OUTPUT,
		@p_count BIGINT OUTPUT,
		@p_out_test BIGINT OUTPUT
	)
AS
BEGIN

	SET NOCOUNT ON;

	DECLARE @temp BIGINT = @p_in_test;
	SET @p_in_test = @p_out_test;
	SET @p_out_test = @temp;

	SELECT
		@p_count = COUNT( account_id )
	FROM
		dynamic.accounts
	WHERE
		closed_on IS NULL;

	SELECT
		account_id,
		account_email,
		nickname,
		nickname_sequence_id
	FROM
		dynamic.accounts
	WHERE
		closed_on IS NULL AND upper_nickname <> @p_filtered_nickname;
    
END;

GO


