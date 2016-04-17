USE testdb;
GO

IF EXISTS (SELECT * FROM sys.objects WHERE type = 'P' AND name = 'no_execute_permission')
	DROP PROCEDURE dynamic.no_execute_permission;
GO

CREATE PROCEDURE dynamic.no_execute_permission 
AS
BEGIN

	SET NOCOUNT ON;
    
	SELECT 0;

END
GO

REVOKE EXECUTE ON OBJECT::dynamic.no_execute_permission FROM testserver;

GO




