USE testdb;
GO

IF EXISTS (SELECT * FROM sys.objects WHERE type = 'P' AND name = 'test_compound_insert')
	DROP PROCEDURE dynamic.test_compound_insert;
GO

CREATE PROCEDURE dynamic.test_compound_insert ( 
  @p_id BIGINT,
  @p_user_data BIGINT
)
AS
BEGIN

	SET NOCOUNT ON;

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
GO




