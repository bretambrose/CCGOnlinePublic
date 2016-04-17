USE testdb;
GO

IF EXISTS (SELECT * FROM sys.objects WHERE type = 'P' AND name = 'do_nothing')
	DROP PROCEDURE dynamic.do_nothing;
GO

CREATE PROCEDURE dynamic.do_nothing 
AS
BEGIN

	SET NOCOUNT ON;

	DECLARE @test BIGINT = 0;
	SELECT
		@test = COUNT( account_id )
	FROM
		dynamic.accounts;
		 
END
GO



