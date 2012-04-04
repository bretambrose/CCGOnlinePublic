import ccg_db_utils

def main():
    ccg_db_utils.CCGRunDBScripts( "Dev", [ "BuildDB/DropTestSchema.sql", "BuildDB/CreateTestSchema.sql", "BuildDB/PopulateTestSchema.sql" ], "RebuildDB" )

main()
