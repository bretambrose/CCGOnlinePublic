import ccg_db_utils

def main():
    ccg_db_utils.CCGRunDBScripts( "CCGDev", [ "BuildDB/DropCCGSchema.sql",
                                              "BuildDB/CreateCCGSchema.sql",
                                              "BuildDB/PopulateCCGSchema.sql" ], "RebuildCCGDB" )

main()
