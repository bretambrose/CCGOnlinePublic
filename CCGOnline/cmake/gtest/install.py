
import os
import shutil
import argparse



def Main():

    parser = argparse.ArgumentParser(description="gtest install script")
    parser.add_argument("--dest", action="store")
    parser.add_argument("--source", action="store")

    args = vars( parser.parse_args() )
    installPath = args[ "dest" ]
    source = args[ "source" ]

    if os.path.exists( installPath ):
        shutil.rmtree( installPath )
    os.makedirs( installPath )

    libPath = os.path.join( installPath, "lib" )
    os.makedirs( libPath )

    includePath = os.path.join( installPath, "include" )

    sourceLibPath = "."
    
    if os.name != 'nt':
        staticExtension = ".a"
    else:
        staticExtension = ".lib"

    for rootDir, dirNames, fileNames in os.walk( sourceLibPath ):
        for fileName in fileNames:
            _, file_extension = os.path.splitext(fileName)
            if file_extension == staticExtension:
                sourceFile = os.path.join(rootDir, fileName)

                shutil.copy(sourceFile, libPath)
    
    sourceIncludePath = os.path.join( source, "include" )
    shutil.copytree( sourceIncludePath, includePath )

    return 0


Main()
