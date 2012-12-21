USE testdb;
GO

IF EXISTS (SELECT * FROM sys.objects WHERE type = 'P' AND name = 'get_all_accounts')
	DROP PROCEDURE dynamic.get_all_accounts;
GO

CREATE PROCEDURE dynamic.get_all_accounts
AS
BEGIN

	SELECT
		account_id,
		account_email,
		nickname,
		nickname_sequence_id
	FROM
		dynamic.accounts
	WHERE
		closed_on IS NULL
	ORDER BY
		account_id ASC;
    
END;

GO

