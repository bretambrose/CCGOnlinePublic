USE testdb;
GO

IF EXISTS (SELECT * FROM sys.objects WHERE type = 'P' AND name = 'unique_constraint_violator')
	DROP PROCEDURE dynamic.unique_constraint_violator;
GO

CREATE PROCEDURE dynamic.unique_constraint_violator 
( 	
	@p_violate_constraint BIT,
	@p_account_count BIGINT OUTPUT
)
AS
BEGIN
    
	IF @p_violate_constraint = 1
	BEGIN

		DECLARE @email VARCHAR(255) = 'bretambrose@gmail.com';
		DECLARE @nickname VARCHAR(32) = 'loser';

		INSERT INTO dynamic.accounts
			(
				account_email,
				upper_account_email,
				password_hash,
				nickname,
				upper_nickname,
				nickname_sequence_id,
				created_on,
				closed_on
			)
		VALUES
			( @email, UPPER( @email ), '', @nickname, UPPER( @nickname ), 1, SYSUTCDATETIME(), NULL );

	END

	SELECT
		@p_account_count = COUNT( account_id )
	FROM
		dynamic.accounts;

	SELECT account_id FROM dynamic.accounts;

END
GO




