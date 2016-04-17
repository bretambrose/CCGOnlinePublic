USE testdb;
GO

IF EXISTS (SELECT * FROM sys.objects WHERE type = 'P' AND name = 'test_compound_insert_count')
	DROP PROCEDURE dynamic.test_compound_insert_count;
GO

CREATE PROCEDURE dynamic.test_compound_insert_count ( 
  @p_id BIGINT
)
AS
BEGIN

	SET NOCOUNT ON;

	SELECT
		COUNT( user_data )
	FROM
		dynamic.test_transactions
	WHERE
		test_transaction_id = @p_id;
    
END
GO



