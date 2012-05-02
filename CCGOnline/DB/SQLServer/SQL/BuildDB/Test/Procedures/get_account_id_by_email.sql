USE testdb;
GO

IF EXISTS (SELECT * FROM sys.objects WHERE type = 'FN' AND name = 'get_account_id_by_email')
	DROP FUNCTION dynamic.get_account_id_by_email;
GO

CREATE FUNCTION dynamic.get_account_id_by_email ( 
  @p_email VARCHAR( 255 )
)
RETURNS BIGINT
AS
BEGIN

	DECLARE @account_id BIGINT = 0;
	SELECT
		@account_id = account_id
	FROM
		dynamic.accounts
	WHERE
		upper_account_email = UPPER( @p_email ) AND closed_on IS NULL;

	RETURN @account_id;
    
END;

GO





