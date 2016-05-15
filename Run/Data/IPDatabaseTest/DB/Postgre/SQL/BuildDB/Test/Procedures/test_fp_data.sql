USE testdb;
GO

IF EXISTS (SELECT * FROM sys.objects WHERE type = 'P' AND name = 'test_fp_data')
	DROP PROCEDURE dynamic.test_fp_data;
GO

CREATE PROCEDURE dynamic.test_fp_data
	(
		@p_float_in REAL,
		@p_double_in FLOAT,
		@p_float_in_out REAL OUTPUT,
		@p_double_in_out FLOAT OUTPUT
	)
AS
BEGIN

	SET NOCOUNT ON;

	SET @p_float_in_out = @p_float_in + @p_float_in_out;
	SET @p_double_in_out = @p_double_in + @p_double_in_out;

	SELECT
		SQRT( CAST ( account_id AS REAL ) ),
		SQRT( CAST ( ( account_id + 1 ) AS FLOAT ) )
	FROM
		dynamic.accounts
	WHERE
		closed_on IS NULL
	ORDER BY
		account_id ASC;
    
END;

GO

