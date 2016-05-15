import re
import subprocess
import os
import random
import time

def SQLException( Exception ):
    def __init__( self, error_code, filename ):
        self.ErrorCode = error_code
        self.FileName = filename

def LoadDBSettings():
    assign_re = re.compile( "(?P<var>\w+)=(?P<value>\S*)" )
    db_kv_pairs = {}

    try:
        db_settings_file = open( "../../Settings/DBSettings.txt", 'r' )

        # read the file, pulling out key value pairs in the form of "var=value"
        line = db_settings_file.readline()
        while line != "":
            result = assign_re.search( line )
            if result != None:
                variable_name = result.group( "var" )
                value = result.group( "value" )
                db_kv_pairs[ variable_name ] = value

            line = db_settings_file.readline()
            
        db_settings_file.close()        
    except IOError:
        print( "ERROR: Unable to find DBSettings.txt" )
    
    return db_kv_pairs


def CleanTempFiles():
    # check for Temp directory existence, create if doesn't exist
    if os.path.exists( "Temp" ) == False:
        os.mkdir( "Temp" )
        return

    for file_name in os.listdir( "Temp" ):
        os.remove( "Temp/" + file_name )
    
def BuildTempSQLFile( sql_script_file_path, log_file_name, procedure_user ):
    full_script_path = "SQL/" + sql_script_file_path
        
    # make sure sql script exists
    if os.path.exists( full_script_path ) == False:
        print( "File not found: " + full_script_path )
        return None

    filename_list = []
    
    if os.path.isdir( full_script_path ):
        for filename in os.listdir( full_script_path ):
            filename_list.append( full_script_path + "/" + filename )
    else:
       filename_list.append( full_script_path )
       
    random_number = random.randint( 0, 1000000 )
    temp_filename = "Temp/" + log_file_name + str( random_number ) + ".sql"

    with open( temp_filename, "w" ) as temp_file:
        for filename in filename_list:
            temp_file.write( ":r " + filename + "\n" )
            
    return temp_filename


def InitLogFiles( log_file_name ):
    # check for Logs directory existence, create if doesn't exist
    if os.path.exists( "Logs" ) == False:
        os.mkdir( "Logs" )

    # Open log files.
    stdout_file_name = "Logs/" + log_file_name + "_output.txt"
    stderr_file_name = "Logs/" + log_file_name + "_error.txt"

    print( "Script output going to: ", stdout_file_name )
    print( "Script errors going to: ", stderr_file_name )

    return ( stdout_file_name, stderr_file_name )


def CCGRunDBScriptsAux( user, sql_script_list, log_file_name, granted_procedure_user ):

    base_start_time = time.clock()
    
    # load our local DB settings
    kv_pairs = LoadDBSettings()

    if "DB" in kv_pairs == False:
        print( "No DB name specific in DBSettings.txt" )
        return

    db_name = kv_pairs[ "DB" ]
    
    # extract the correct db username
    username_key = user + "Account"
    if username_key in kv_pairs == False:
        print( "Unknown DB User: " + user )
        return

    username = kv_pairs[ username_key ]

    # extract the correct password
    username_password_key = user + "Password"
    if username_password_key in kv_pairs == False:
        print( "No password entry for DB user: " + user )
        return
    
    password = kv_pairs[ username_password_key ]

    print( "Login: " + username + "/" + password );

    procedure_user = ""
    if granted_procedure_user != "":
        procedure_user_key = granted_procedure_user + "Account"
        if procedure_user_key in kv_pairs == False:
            print( "Unknown Procedure Grantee: " + granted_procedure_user )
            return

        procedure_user = kv_pairs[ procedure_user_key ]
        
    # assumes being run from the DB/Python subdirectory
    old_directory = os.getcwd()

    try:
        os.chdir( ".." )

        CleanTempFiles()
        stdout_file_name, stderr_file_name = InitLogFiles( log_file_name )
                        
        with open( stdout_file_name, "w" ) as test_log_file, open( stderr_file_name, "w" ) as test_error_file:

            for sql_script_file_path in sql_script_list:

                temp_filename = BuildTempSQLFile( sql_script_file_path, log_file_name, procedure_user )
                if temp_filename != None:                       
                    print( "Processing " + sql_script_file_path + " using temp file: " + temp_filename )
                    test_log_file.write( "\n***************************************************************\n" )
                    test_log_file.write( "Processing file: " + sql_script_file_path + "\n\n" )
                    test_log_file.flush()
                    
                    # build the argument list
                    command_line_args = "-S " + db_name + " -U " + username + " -P " + password + " -i " + temp_filename
                    
                    print( "Command line args: " + command_line_args )
     
                    # Execute the SQL script
                    return_code = subprocess.call( "sqlcmd.exe " + command_line_args, stdout = test_log_file, stderr = test_error_file )
                    if return_code != 0:
                        test_log_file.write( "SQLServer Error: " + str( return_code ) + "\n" )
                        test_log_file.write( "*****ABORTING script execution*****\n" )
                        test_log_file.flush()
                        raise SQLException( return_code, sql_script_file_path )

    except SQLException as sql_exception:
        print( "SQL Error ( " + str( sql_exception.ErrorCode ) + " ) while executing file: " + sql_exception.FileName )
        raise sql_exception
    
    finally:
        end_time = time.clock()
        print( "Total Time: " + str( end_time - base_start_time ) + " seconds" )
        os.chdir( old_directory )

        
def CCGRunDBScript( user, sql_script_path, script_type, log_file_name, granted_procedure_user = "" ):
    CCGRunDBScripts( user, [ ( sql_script_path, script_type ) ], log_file_name, granted_procedure_user )


def CCGRunDBScripts( user, sql_script_path_list, log_file_name, granted_procedure_user = "", exit_immediately = False ):

    try:
        CCGRunDBScriptsAux( user, sql_script_path_list, log_file_name, granted_procedure_user )
        print( "Success!" )
       
    except:
        print( "There were errors =(" )
        
    finally:
        if exit_immediately == False:
            input( "Press Enter to exit" )
    




