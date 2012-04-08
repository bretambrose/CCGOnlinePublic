import re
import subprocess
import os
import random
import time

def LoadDBSettings():
    assign_re = re.compile( "(?P<var>\w+)=(?P<value>\S*)" )
    db_kv_pairs = {}

    try:
        db_settings_file = open( "../DBSettings.txt", 'r' )

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

    
def BuildTempSQLFile( sql_script_file_path, log_file_name ):
    full_script_path = "SQL/" + sql_script_file_path
        
    # make sure sql script exists
    if os.path.exists( full_script_path ) == False:
        print( "SQL Script not found: " + full_script_path )
        return None

    random_number = random.randint( 0, 1000000 )
    temp_filename = "Temp/" + log_file_name + str( random_number ) + ".sql"

    with open( temp_filename, "w" ) as temp_file:

        temp_file.write( "set feedback off;\n\n" )        
        # temp_file.write( "DECLARE\n" )
        # temp_file.write( "BEGIN\n" )
        temp_file.write( "@" + full_script_path + ";\n" )
        # temp_file.write( "DBMS_OUTPUT.PUT_LINE( 'test' );\n" )
        # temp_file.write( "END;\n" )
        temp_file.write( "/\n" )
        temp_file.write( "\nquit\n" )

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


def CCGRunDBScriptsAux( user, sql_script_path_list, log_file_name ):

    base_start_time = time.clock()
    
    # load our local DB settings
    kv_pairs = LoadDBSettings()

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
    
    # assumes being run from the DB/Python subdirectory
    old_directory = os.getcwd()

    try:
        os.chdir( ".." )

        stdout_file_name, stderr_file_name = InitLogFiles( log_file_name )
        
        with open( stdout_file_name, "w" ) as test_log_file, open( stderr_file_name, "w" ) as test_error_file:

            for sql_script_file_path in sql_script_path_list:

                CleanTempFiles()
                temp_filename = BuildTempSQLFile( sql_script_file_path, log_file_name )
                if temp_filename != None:                       
                    print( "Processing using temp file: " + temp_filename )
                    
                    # build the argument list
                    command_line_args = "-S " + username + "/" + password + " @" + temp_filename
                    print( "Command line args: " + command_line_args )
     
                    # Execute the SQL script
                    subprocess.call( "SQLPlus.exe " + command_line_args, stdout = test_log_file, stderr = test_error_file )                   
 
    finally:
        end_time = time.clock()
        print( "Total Time: " + str( end_time - base_start_time ) + " seconds" )
        os.chdir( old_directory )

        
def CCGRunDBScript( user, sql_script_path, log_file_name ):
    CCGRunDBScripts( user, [ sql_script_path ], log_file_name )


def CCGRunDBScripts( user, sql_script_path_list, log_file_name ):

    try:
        CCGRunDBScriptsAux( user, sql_script_path_list, log_file_name )
        print( "Success!" )

    except:
        print( "There were errors =(" )
        
    finally:
        input( "Press Enter to exit" )
    




