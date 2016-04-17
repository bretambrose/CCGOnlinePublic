USE testdb;
GO

IF EXISTS (SELECT * FROM sys.objects WHERE type = 'P' AND name = 'invalid_result_conversion_procedure')
	DROP PROCEDURE dynamic.invalid_result_conversion_procedure;
GO

CREATE PROCEDURE dynamic.invalid_result_conversion_procedure 
( 	
	@p_do_invalid_conversion BIT
)
AS
BEGIN

	SET NOCOUNT ON;
    
	IF @p_do_invalid_conversion = 1
	BEGIN
		
		SELECT 
			CASE WHEN account_id = 3 THEN 'Bret' ELSE CAST ( account_id AS VARCHAR(10) ) END
		FROM 
			dynamic.accounts 
		ORDER BY 
			account_id ASC;

	END
	ELSE
	BEGIN

		SELECT CAST( account_id AS REAL ) FROM dynamic.accounts ORDER BY account_id ASC;

	END

END
GO




