USE testdb;

IF EXISTS (SELECT * FROM sys.objects WHERE type = 'P' AND name = 'get_account_by_email')
	DROP PROCEDURE dynamic.get_account_by_email;
GO

CREATE PROCEDURE dynamic.get_account_by_email ( 
  @p_email VARCHAR( 255 )
)
AS
BEGIN

  SELECT
    account_id,
    nickname,
    nickname_sequence_id
  FROM
    dynamic.accounts
  WHERE
    upper_account_email = UPPER( @p_email ) AND closed_on IS NULL;
    
END
GO




