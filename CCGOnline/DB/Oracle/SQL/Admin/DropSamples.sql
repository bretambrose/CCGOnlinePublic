DECLARE 
  user_count INTEGER;
  type username_table_type is table of varchar2(32);
  username_table username_table_type;  
BEGIN
  username_table := username_table_type('HR', 'IX', 'OE', 'PM', 'SCOTT', 'SH');

  FOR elem IN 1 .. username_table.count LOOP
    SELECT
      COUNT(*) into user_count
    FROM
      dba_users
    WHERE
      USERNAME = username_table(elem);
      
    IF user_count = 1 THEN
      EXECUTE IMMEDIATE 'DROP USER ' || username_table(elem) || ' CASCADE';
    END IF;  
  END LOOP;

END;



