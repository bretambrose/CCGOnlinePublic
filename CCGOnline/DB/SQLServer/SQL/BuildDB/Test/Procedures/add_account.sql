USE testdb;
GO

IF EXISTS (SELECT * FROM sys.objects WHERE type = 'P' AND name = 'add_account')
	DROP PROCEDURE dynamic.add_account;
GO

CREATE PROCEDURE dynamic.add_account ( 
  @p_email VARCHAR(255),
  @p_nickname VARCHAR(32),
  @p_password_hash VARCHAR(32)
)
AS
BEGIN
 
	DECLARE @upper_nickname VARCHAR(32) = UPPER( @p_nickname );
	DECLARE @nickname_seq_id BIGINT;
 
	SELECT
		@nickname_seq_id = COUNT( account_id ) + 1
	FROM
		dynamic.accounts
	WHERE
		upper_nickname = @upper_nickname;
    
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
		( @p_email, UPPER( @p_email ), @p_password_hash, @p_nickname, UPPER( @p_nickname ), @nickname_seq_id, SYSUTCDATETIME(), NULL );
  
END
GO



