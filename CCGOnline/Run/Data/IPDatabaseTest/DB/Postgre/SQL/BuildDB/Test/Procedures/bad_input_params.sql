USE testdb;
GO

IF EXISTS (SELECT * FROM sys.objects WHERE type = 'P' AND name = 'bad_input_params')
	DROP PROCEDURE dynamic.bad_input_params;
GO

CREATE PROCEDURE dynamic.bad_input_params 
( 	
	@p_test REAL
)
AS
BEGIN

	SET NOCOUNT ON;
    
	RETURN

END
GO




