USE testdb;
GO

IF EXISTS (SELECT * FROM sys.objects WHERE type = 'P' AND name = 'missing_column_procedure')
	DROP PROCEDURE dynamic.missing_column_procedure;
GO

CREATE PROCEDURE dynamic.missing_column_procedure 
( 	
	@p_missing_column BIT
)
AS
BEGIN

	SET NOCOUNT ON;
    
	IF @p_missing_column = 1
	BEGIN
		
		SELECT account_id FROM dynamic.accounts ORDER BY account_id ASC;

	END
	ELSE
	BEGIN

		SELECT account_id, 1 FROM dynamic.accounts ORDER BY account_id ASC;

	END

END
GO




