import ccg_db_utils

def main():
    ccg_db_utils.CCGRunDBScript( "System", "Admin/OneTimeInitialize.sql", "OneTimeInitialize" )

main()
