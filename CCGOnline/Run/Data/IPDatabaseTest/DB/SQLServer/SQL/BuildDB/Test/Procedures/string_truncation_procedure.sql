USE testdb;
GO

IF EXISTS (SELECT * FROM sys.objects WHERE type = 'P' AND name = 'string_truncation_procedure')
	DROP PROCEDURE dynamic.string_truncation_procedure;
GO

CREATE PROCEDURE dynamic.string_truncation_procedure 
( 	
	@p_do_truncation BIT
)
AS
BEGIN

	SET NOCOUNT ON;
    
	IF @p_do_truncation = 1
	BEGIN
		
		SELECT 
			CASE WHEN account_id = 2 THEN account_email + 'aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa' ELSE account_email END
		FROM 
			dynamic.accounts 
		ORDER BY 
			account_id ASC;

	END
	ELSE
	BEGIN

		SELECT account_email FROM dynamic.accounts ORDER BY account_id ASC;

	END

END
GO




