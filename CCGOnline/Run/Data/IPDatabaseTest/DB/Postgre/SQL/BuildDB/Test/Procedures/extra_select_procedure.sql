USE testdb;
GO

IF EXISTS (SELECT * FROM sys.objects WHERE type = 'P' AND name = 'extra_select_procedure')
	DROP PROCEDURE dynamic.extra_select_procedure;
GO

CREATE PROCEDURE dynamic.extra_select_procedure 
( 	
	@p_extra_select BIT,
	@p_account_count BIGINT OUTPUT
)
AS
BEGIN

	SET NOCOUNT ON;
 
	SELECT
		@p_account_count = COUNT( account_id )
	FROM
		dynamic.accounts;
    
	IF @p_extra_select = 1
	BEGIN
		
		SELECT account_id FROM dynamic.accounts;

	END

END
GO




