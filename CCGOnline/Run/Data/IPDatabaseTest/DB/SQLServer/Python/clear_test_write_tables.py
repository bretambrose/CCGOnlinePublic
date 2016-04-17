import ccg_sqlserver_utils

def main():
    ccg_sqlserver_utils.CCGRunDBScripts( "TestDev", [ "BuildDB/Test/ClearTestWriteTables.sql" ], "ClearTestWriteTables", "", True )

main()
