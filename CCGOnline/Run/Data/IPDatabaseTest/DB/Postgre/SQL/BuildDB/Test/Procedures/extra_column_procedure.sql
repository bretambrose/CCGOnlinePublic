USE testdb;
GO

IF EXISTS (SELECT * FROM sys.objects WHERE type = 'P' AND name = 'extra_column_procedure')
	DROP PROCEDURE dynamic.extra_column_procedure;
GO

CREATE PROCEDURE dynamic.extra_column_procedure 
( 	
	@p_extra_column BIT
)
AS
BEGIN

	SET NOCOUNT ON;
    
	IF @p_extra_column = 1
	BEGIN
		
		SELECT account_id, 1 FROM dynamic.accounts ORDER BY account_id ASC;

	END
	ELSE
	BEGIN

		SELECT account_id FROM dynamic.accounts ORDER BY account_id ASC;

	END

END
GO




