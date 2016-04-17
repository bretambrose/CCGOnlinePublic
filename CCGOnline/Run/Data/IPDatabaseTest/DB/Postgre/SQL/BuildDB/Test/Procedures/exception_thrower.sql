USE testdb;
GO

IF EXISTS (SELECT * FROM sys.objects WHERE type = 'P' AND name = 'exception_thrower')
	DROP PROCEDURE dynamic.exception_thrower;
GO

CREATE PROCEDURE dynamic.exception_thrower 
( 	
	@p_throw_exception BIT,
	@p_account_count BIGINT OUTPUT
)
AS
BEGIN

	SET NOCOUNT ON;
    
	IF @p_throw_exception = 1
	BEGIN
		THROW 50000, 'Test Throw Error', 1;

	END

	SELECT
		@p_account_count = COUNT( account_id )
	FROM
		dynamic.accounts;

	SELECT account_id FROM dynamic.accounts;

END
GO




