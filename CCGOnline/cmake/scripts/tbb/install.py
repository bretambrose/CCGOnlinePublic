
import os
import shutil
import argparse



def Main():

    parser = argparse.ArgumentParser(description="tbb install script")
    parser.add_argument("--config", action="store_true")

    args = vars( parser.parse_args() )
    config = args[ "config" ] or "debug"

    installPath = os.path.join( "..", "external", "tbb" )
    if os.path.exists( installPath ):
        shutil.rmtree( installPath )

    if os.path.exists( installPath ) == False:
        os.makedirs( installPath )

    includePath = os.path.join( installPath, "include" )
    if os.path.exists( includePath ) == False:
        os.makedirs( includePath )

        sourceIncludePath = os.path.join( "include" )

        copySourceTree = os.path.join( sourceIncludePath, "tbb" )
        copyDestTree = os.path.join( includePath, "tbb" )

        print ( "SourceTree = " + str(copySourceTree) + " (" + os.path.abspath(copySourceTree) + ")")
        print ( "DestTree = " + str(copyDestTree) + " (" + os.path.abspath(copyDestTree) + ")")

        shutil.copytree( copySourceTree, copyDestTree )


    libPath = os.path.join( installPath, "lib" )
    if os.path.exists( libPath ) == False:
        os.makedirs( libPath )


    sourceLibPath = os.path.join( "build", "tbb_" + config )

    for rootDir, dirNames, fileNames in os.walk( sourceLibPath ):
        for fileName in fileNames:
            _, file_extension = os.path.splitext(fileName)
            if file_extension == '.so':
                sourceFile = os.path.join(rootDir, fileName)

                shutil.copy(sourceFile, libPath)


    return 0


Main()
