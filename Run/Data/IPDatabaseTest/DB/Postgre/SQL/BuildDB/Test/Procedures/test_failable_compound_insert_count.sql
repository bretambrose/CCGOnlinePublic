USE testdb;
GO

IF EXISTS (SELECT * FROM sys.objects WHERE type = 'P' AND name = 'test_failable_compound_insert_count')
	DROP PROCEDURE dynamic.test_failable_compound_insert_count;
GO

CREATE PROCEDURE dynamic.test_failable_compound_insert_count ( 
  @p_id BIGINT,
  @p_should_fail BIT
)
AS
BEGIN

	SET NOCOUNT ON;

	IF @p_should_fail = 0
	BEGIN
		SELECT
			COUNT( user_data )
		FROM
			dynamic.test_transactions
		WHERE
			test_transaction_id = @p_id;
	END
	ELSE
	BEGIN
		THROW 50000, 'Test Throw Error', 1;
	END
    
END
GO



