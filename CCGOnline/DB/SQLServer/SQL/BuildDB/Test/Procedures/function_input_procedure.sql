USE testdb;
GO

IF EXISTS (SELECT * FROM sys.objects WHERE type = 'P' AND name = 'function_input_procedure')
	DROP PROCEDURE dynamic.function_input_procedure;
GO

CREATE PROCEDURE dynamic.function_input_procedure 
( 	
	@p_test1 BIGINT,
	@p_test2 BIGINT
)
AS
BEGIN
    
	RETURN

END
GO




