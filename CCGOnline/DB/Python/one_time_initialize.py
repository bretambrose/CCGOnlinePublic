import ccg_db_utils

def main():
    # ccg_db_utils.CCGRunDBScripts( "System", [ "Admin/DropSamples.sql", "Admin/CCGUsers.sql", "Admin/CCGTablespaces.sql" ], "OneTimeInitialize" )
    ccg_db_utils.CCGRunDBScripts( "System", [ "Admin/DropSamples.sql", "Admin/OneTimeTeardown.sql", "Admin/OneTimeInitialize.sql" ], "OneTimeInitialize" )

main()
