USE testdb;
GO

IF EXISTS (SELECT * FROM sys.objects WHERE type = 'P' AND name = 'too_many_results_procedure')
	DROP PROCEDURE dynamic.too_many_results_procedure;
GO

CREATE PROCEDURE dynamic.too_many_results_procedure 
( 	
	@p_extra_select BIT
)
AS
BEGIN

	SET NOCOUNT ON;
    
	SELECT account_id FROM dynamic.accounts ORDER BY account_id ASC;

	IF @p_extra_select = 1
	BEGIN
		
		SELECT account_id FROM dynamic.accounts ORDER BY account_id ASC;

	END

END
GO




