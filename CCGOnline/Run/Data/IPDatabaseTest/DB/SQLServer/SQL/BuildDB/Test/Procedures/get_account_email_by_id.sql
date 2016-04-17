USE testdb;
GO

IF EXISTS (SELECT * FROM sys.objects WHERE type = 'FN' AND name = 'get_account_email_by_id')
	DROP FUNCTION dynamic.get_account_email_by_id;
GO

CREATE FUNCTION dynamic.get_account_email_by_id
	(
		@p_account_id BIGINT
	)
RETURNS VARCHAR(255)
AS
BEGIN

	DECLARE @account_email VARCHAR(255) = '';

	SELECT
		@account_email = account_email
	FROM
		dynamic.accounts
	WHERE
		closed_on IS NULL AND account_id = @p_account_id;

	RETURN @account_email;
    
END;

GO

