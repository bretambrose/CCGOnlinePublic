import ccg_sqlserver_utils

def main():
    ccg_sqlserver_utils.CCGRunDBScripts( "TestDev", [ "BuildDB/Test/DropTestForeignKeys.sql",
                                                      "BuildDB/Test/DropTestObjects.sql",
                                                      "BuildDB/Test/CreateTestTables.sql",
                                                      "BuildDB/Test/CreateTestForeignKeys.sql", 
                                                      "BuildDB/Test/Procedures", 
                                                      "BuildDB/Test/PopulateTestTables.sql" ], "RebuildTestDB", "TestServer", True )

main()
