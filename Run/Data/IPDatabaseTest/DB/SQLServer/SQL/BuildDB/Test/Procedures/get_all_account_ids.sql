USE testdb;
GO

IF EXISTS (SELECT * FROM sys.objects WHERE type = 'P' AND name = 'get_all_account_ids')
	DROP PROCEDURE dynamic.get_all_account_ids;
GO

CREATE PROCEDURE dynamic.get_all_account_ids
AS
BEGIN

	SET NOCOUNT ON;

	SELECT
		account_id
	FROM
		dynamic.accounts
	WHERE
		closed_on IS NULL
	ORDER BY
		account_id ASC;
    
END;

GO

