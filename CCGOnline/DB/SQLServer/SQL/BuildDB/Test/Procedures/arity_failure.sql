USE testdb;
GO

IF EXISTS (SELECT * FROM sys.objects WHERE type = 'P' AND name = 'arity_failure')
	DROP PROCEDURE dynamic.arity_failure;
GO

CREATE PROCEDURE dynamic.arity_failure 
( 	
	@p_id BIGINT,
	@p_nickname VARCHAR( 32 ),
	@p_nickname_seq_id INT
)
AS
BEGIN

	SET NOCOUNT ON;
    
	RETURN

END
GO




