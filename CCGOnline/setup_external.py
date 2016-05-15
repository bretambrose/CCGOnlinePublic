import os
import shutil
import urllib.request
import subprocess
import zipfile


def GetPackages():
    return { "tbb" : { "url" : "https://www.threadingbuildingblocks.org/sites/default/files/software_releases/windows/tbb44_20151115oss_win_0.zip",
                       "file" : "tbb.zip",
                       "destination" : "External",
                       "renameTarget" : "tbb44_20151115oss" },
             "loki" : { "url" : "http://sourceforge.net/projects/loki-lib/files/Loki/Loki%200.1.7/loki-0.1.7.zip",
                        "file" : "loki-0.1.7.zip",
                        "destination" : "External",
                        "renameTarget" : "loki-0.1.7" } }


def Main():

    if os.path.exists( "tmp" ):
        shutil.rmtree( "tmp" )

    while os.path.exists( "tmp" ):
        time.sleep(1)
        
    os.mkdir( "tmp" )
    
    packages = GetPackages()
            
    for packageName, packageData in packages.items():
        destPath = os.path.join( packageData[ "destination" ], packageName )
        print( "destPath = " + destPath )
        if os.path.exists( destPath ):
            shutil.rmtree( destPath )
            
    for packageName, packageData in packages.items():
        downloadUrl = packageData[ "url" ]
        response = urllib.request.urlopen( downloadUrl )
        data = response.read()

        zipFilename = os.path.join( "tmp", packageData[ "file" ] )
        targetFile = open( zipFilename, 'wb' )
        targetFile.write( data )
        targetFile.close()

        with zipfile.ZipFile( zipFilename, "r" ) as z:
            z.extractall( "tmp" )

        destPath = os.path.join( packageData[ "destination" ], packageName )
        sourcePath = os.path.join( "tmp", packageData[ "renameTarget" ] )

        print( "Move " + sourcePath + " to " + destPath )
        shutil.move( sourcePath, destPath )


    if os.path.exists( "googletest" ):
        shutil.rmtree( "googletest" )

    while os.path.exists( "googletest" ):
        time.sleep(1)
        
    subprocess.check_call( "git clone https://github.com/google/googletest.git" )
    
Main()
