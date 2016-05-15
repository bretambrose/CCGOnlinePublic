USE testdb;
GO

IF EXISTS (SELECT * FROM sys.objects WHERE type = 'P' AND name = 'bad_result_set_conversion')
	DROP PROCEDURE dynamic.bad_result_set_conversion;
GO

CREATE PROCEDURE dynamic.bad_result_set_conversion 
AS
BEGIN

	SET NOCOUNT ON;
    
	SELECT 'Bret';

END
GO




