USE testdb;
GO

IF EXISTS (SELECT * FROM sys.objects WHERE type = 'P' AND name = 'skip_select_procedure')
	DROP PROCEDURE dynamic.skip_select_procedure;
GO

CREATE PROCEDURE dynamic.skip_select_procedure 
( 	
	@p_skip_select BIT,
	@p_account_count BIGINT OUTPUT
)
AS
BEGIN

	SET NOCOUNT ON;
 
	SELECT
		@p_account_count = COUNT( account_id )
	FROM
		dynamic.accounts;
    
	IF @p_skip_select = 0
	BEGIN
		
		SELECT account_id FROM dynamic.accounts;

	END

END
GO




