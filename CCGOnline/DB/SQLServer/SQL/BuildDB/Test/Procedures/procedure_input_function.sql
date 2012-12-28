USE testdb;
GO

IF EXISTS (SELECT * FROM sys.objects WHERE type = 'FN' AND name = 'procedure_input_function')
	DROP FUNCTION dynamic.procedure_input_function;
GO

CREATE FUNCTION dynamic.procedure_input_function
	(
		@p_id BIGINT
	)
RETURNS BIGINT
AS
BEGIN

	RETURN 0;
    
END;

GO

