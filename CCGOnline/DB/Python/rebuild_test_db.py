import ccg_db_utils

def main():
    ccg_db_utils.CCGRunDBScripts( "TestDev", [ ( "BuildDB/Test/DropTestForeignKeys.sql", "PL" ),
                                               ( "BuildDB/Test/DropTestSchema.sql", "PL" ),
                                               ( "BuildDB/Test/CreateTestSchema.sql", "SQL" ),
                                               ( "BuildDB/Test/CreateTestForeignKeys.sql", "SQL" ),
                                               ( "BuildDB/Test/Procedures", "PROC" ),
                                               ( "BuildDB/Test/PopulateTestSchema.sql", "PL" ), ], "RebuildTestDB" )

main()
