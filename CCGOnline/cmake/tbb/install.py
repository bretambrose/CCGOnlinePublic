
import os
import shutil
import argparse



def Main():

    parser = argparse.ArgumentParser(description="tbb install script")
    parser.add_argument("--config", action="store")
    parser.add_argument("--dest", action="store")

    args = vars( parser.parse_args() )
    config = args[ "config" ] or "debug"
    installPath = args[ "dest" ]

    if os.path.exists( installPath ):
        shutil.rmtree( installPath )
    os.makedirs( installPath )

    libPath = os.path.join( installPath, "lib" )
    os.makedirs( libPath )

    includePath = os.path.join( installPath, "include" )

    sourceLibPath = os.path.join( "build", "tbb_" + config )

    if os.name != 'nt':
        sharedExtension = ".so"
    else:
        sharedExtension = ".dll"
    
    for rootDir, dirNames, fileNames in os.walk( sourceLibPath ):
        for fileName in fileNames:
            _, file_extension = os.path.splitext(fileName)
            if file_extension == sharedExtension:
                sourceFile = os.path.join(rootDir, fileName)

                shutil.copy(sourceFile, libPath)


    sourceIncludePath = os.path.join( "include" )
    shutil.copytree( sourceIncludePath, includePath )

    os.remove( os.path.join( includePath, "index.html" ) )

    return 0


Main()
