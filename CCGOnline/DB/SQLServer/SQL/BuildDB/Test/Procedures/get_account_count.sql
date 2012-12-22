USE testdb;
GO

IF EXISTS (SELECT * FROM sys.objects WHERE type = 'FN' AND name = 'get_account_count')
	DROP FUNCTION dynamic.get_account_count;
GO

CREATE FUNCTION dynamic.get_account_count
	(
		@p_filter_nickname VARCHAR(32)
	)
RETURNS BIGINT
AS
BEGIN

	DECLARE @account_count BIGINT = 0;
	SELECT
		@account_count = COUNT( account_id )
	FROM
		dynamic.accounts
	WHERE
		closed_on IS NULL AND upper_nickname <> UPPER( @p_filter_nickname );

	RETURN @account_count;
    
END;

GO

