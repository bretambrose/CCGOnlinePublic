
import os
import shutil
import argparse



def Main():

    parser = argparse.ArgumentParser(description="loki install script")
    parser.add_argument("--dest", action="store")

    args = vars( parser.parse_args() )
    installPath = args[ "dest" ]

    if os.path.exists( installPath ):
        shutil.rmtree( installPath )
    os.makedirs( installPath )

    libPath = os.path.join( installPath, "lib" )
    os.makedirs( libPath )

    includePath = os.path.join( installPath, "include" )

    sourceLibPath = os.path.join( "lib" )
    
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


    sourceIncludePath = os.path.join( "include" )
    shutil.copytree( sourceIncludePath, includePath )

    os.remove( os.path.join( includePath, "Makefile" ) )

    return 0


Main()
