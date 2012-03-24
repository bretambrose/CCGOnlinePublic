import re
import subprocess
import os

def LoadDBSettings():
    assign_re = re.compile( "(?P<var>\w+)=(?P<value>\w*)" )
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

def CCGRunDBScript( user, sql_script_path, log_file_name ):
    CCGRunDBScripts( user, [ sql_script_path ], log_file_name )
    
def CCGRunDBScripts( user, sql_script_path_list, log_file_name ):
    
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
        
    # assumes being run from the DB/Python subdirectory
    old_directory = os.getcwd()

    try:
        os.chdir( ".." )

        # check for Logs directory existence, create if doesn't exist
        if os.path.exists( "Logs" ) == False:
            os.mkdir( "Logs" )

        # Open log files.
        # ( base, ext ) = os.path.splitext( sql_script_path )
        # sql_file_name = os.path.basename( base )
        stdout_file_name = "Logs/" + log_file_name + "_output.txt"
        stderr_file_name = "Logs/" + log_file_name + "_error.txt"

        print( "Script output going to: ", stdout_file_name )
        print( "Script errors going to: ", stderr_file_name )

        with open( stdout_file_name, "w" ) as test_log_file, open( stderr_file_name, "w" ) as test_error_file:

            for script_path in sql_script_path_list:

                full_script_path = "SQL/" + script_path
                
                # make sure sql script exists
                if os.path.exists( full_script_path ) == False:
                    print( "SQL Script not found: " + full_script_path )
                    return

                print( "Processing script: ", script_path )
                test_log_file.write( "*********************************************************************\n" )
                test_log_file.write( "Processing script: " + script_path + "\n\n" )
                test_log_file.flush()
                
                # build the argument list
                command_line_args = "-S " + username + "/" + password + " @" + full_script_path

                # Execute the SQL script
                subprocess.call( "SQLPlus.exe " + command_line_args, stdout = test_log_file, stderr = test_error_file )

    finally:
        os.chdir( old_directory )

    print( "Finished!" )
    input( "Press Enter to exit" )

