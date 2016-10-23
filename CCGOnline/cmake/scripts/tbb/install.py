
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

    if os.path.exists( installPath ) == False:
        os.makedirs( installPath )

    sourceLibPath = os.path.join( "build", "tbb_" + config )

    for rootDir, dirNames, fileNames in os.walk( sourceLibPath ):
        for fileName in fileNames:
            _, file_extension = os.path.splitext(fileName)
            if file_extension == '.so':
                sourceFile = os.path.join(rootDir, fileName)

                shutil.copy(sourceFile, installPath)


    return 0


Main()
