USE testdb;
GO

IF EXISTS (SELECT * FROM sys.objects WHERE type = 'FN' AND name = 'bad_function_return')
	DROP FUNCTION dynamic.bad_function_return;
GO

CREATE FUNCTION dynamic.bad_function_return
	(
		@p_id BIGINT
	)
RETURNS VARCHAR(32)
AS
BEGIN

	RETURN 'Bret';
    
END;

GO

