USE testdb;
GO

IF EXISTS (SELECT * FROM sys.objects WHERE type = 'P' AND name = 'test_failable_compound_insert')
	DROP PROCEDURE dynamic.test_failable_compound_insert;
GO

CREATE PROCEDURE dynamic.test_failable_compound_insert ( 
  @p_id BIGINT,
  @p_user_data BIGINT,
  @p_should_fail BIT
)
AS
BEGIN

	SET NOCOUNT ON;

	IF @p_should_fail = 0
	BEGIN
		INSERT INTO dynamic.test_transactions
			(
				test_transaction_id,
				user_data
			)
		VALUES
			(
				@p_id,
				@p_user_data
			);
	END
	ELSE
	BEGIN
		THROW 50000, 'Test Throw Error', 1;
	END
    
END
GO




