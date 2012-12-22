USE testdb;
GO

IF EXISTS (SELECT * FROM sys.objects WHERE type = 'P' AND name = 'test_boolean_data')
	DROP PROCEDURE dynamic.test_boolean_data;
GO

CREATE PROCEDURE dynamic.test_boolean_data
	(
		@p_in BIT,
		@p_in_out BIT OUTPUT
	)
AS
BEGIN

	SET @p_in_out = @p_in ^ @p_in_out;

	SELECT
		CASE WHEN ( account_id = 1 ) THEN 1 ELSE 0 END
	FROM
		dynamic.accounts
	WHERE
		closed_on IS NULL
	ORDER BY
		account_id ASC;
    
END;

GO

