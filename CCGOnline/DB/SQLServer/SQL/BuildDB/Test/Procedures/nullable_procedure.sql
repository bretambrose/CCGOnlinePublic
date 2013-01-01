USE testdb;
GO

IF EXISTS (SELECT * FROM sys.objects WHERE type = 'P' AND name = 'nullable_procedure')
	DROP PROCEDURE dynamic.nullable_procedure;
GO

CREATE PROCEDURE dynamic.nullable_procedure ( 
	@p_null_account_id BIGINT,
	@p_string_in_out VARCHAR( 32 ) OUTPUT,
	@p_wstring_in_out NVARCHAR( 32 ) OUTPUT
)
AS
BEGIN

	SET NOCOUNT ON;

	DECLARE @temp VARCHAR( 32 );
	SET @temp = @p_wstring_in_out ;
	SET @p_wstring_in_out = @p_string_in_out;
	SET @p_string_in_out = @temp;

	SELECT
		CASE WHEN account_id = @p_null_account_id THEN NULL ELSE account_id END,
		CASE WHEN account_id = @p_null_account_id THEN NULL ELSE account_email END
	FROM
		dynamic.accounts
	ORDER BY
		account_id ASC;
    
END
GO




