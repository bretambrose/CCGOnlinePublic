import ccg_sqlserver_utils

def main():
    ccg_sqlserver_utils.CCGRunDBScripts( "System", [ "Admin/OneTimeCleanup.sql",
                                                     "Admin/CreateDBs.sql",
                                                     "Admin/CreateLogins.sql",
                                                     "Admin/CreateSchemas.sql",
                                                     "Admin/CreateUsers.sql" ], "OneTimeSetup" )

main()
