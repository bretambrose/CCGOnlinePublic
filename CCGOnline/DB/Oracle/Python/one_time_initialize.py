import ccg_db_utils

def main():
    ccg_db_utils.CCGRunDBScripts( "System", [ ( "Admin/DropSamples.sql", "PL" ),
                                              ( "Admin/OneTimeTeardown.sql", "PL" ),
                                              ( "Admin/OneTimeInitialize.sql", "PL" ) ], "OneTimeInitialize" )

main()
