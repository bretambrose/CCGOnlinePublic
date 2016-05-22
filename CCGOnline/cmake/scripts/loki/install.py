
import os
import shutil


def Main():

    installPath = os.path.join( "..", "external", "loki" )
    if os.path.exists( installPath ):
        shutil.rmtree( installPath )

    if os.path.exists( installPath ) == False:
        os.makedirs( installPath )

    includePath = os.path.join( installPath, "include" )
    if os.path.exists( includePath ) == False:
        os.makedirs( includePath )

        sourceIncludePath = os.path.join( "include" )

        copySourceTree = os.path.join( sourceIncludePath, "loki" )
        copyDestTree = os.path.join( includePath, "loki" )

        shutil.copytree( copySourceTree, copyDestTree )


    libPath = os.path.join( installPath, "lib" )
    if os.path.exists( libPath ) == False:
        os.makedirs( libPath )


    sourceLibPath = os.path.join( "lib")

    for rootDir, dirNames, fileNames in os.walk( sourceLibPath ):
        for fileName in fileNames:
            _, file_extension = os.path.splitext(fileName)
            if file_extension == '.a':
                sourceFile = os.path.join(rootDir, fileName)

                shutil.copy(sourceFile, libPath)


    return 0


Main()
