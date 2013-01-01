USE testdb;
GO

IF EXISTS (SELECT * FROM sys.objects WHERE type = 'FN' AND name = 'get_account_by_nickname')
	DROP FUNCTION dynamic.get_account_by_nickname;
GO

CREATE FUNCTION dynamic.get_account_by_nickname
	(
		@p_nickname NVARCHAR(32)
	)
RETURNS BIGINT
AS
BEGIN

	DECLARE @account_id BIGINT;

	SELECT
		@account_id = account_id
	FROM
		dynamic.accounts
	WHERE
		upper_nickname = UPPER( @p_nickname );

	RETURN @account_id;
    
END;

GO

