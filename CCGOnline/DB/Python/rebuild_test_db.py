import ccg_db_utils

def main():
    ccg_db_utils.CCGRunDBScripts( "TestDev", [ "BuildDB/Test/DropTestForeignKeys.sql",
                                               "BuildDB/Test/DropTestSchema.sql",
                                               "BuildDB/Test/CreateTestSchema.sql",
                                               "BuildDB/Test/CreateTestForeignKeys.sql",
                                               "BuildDB/Test/PopulateTestSchema.sql" ], "RebuildTestDB" )

main()
