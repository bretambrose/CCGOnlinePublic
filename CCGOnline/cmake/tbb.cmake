
set(TBB_SOURCE_DIR "${CMAKE_SOURCE_DIR}/tbb")

if(PLATFORM_WINDOWS)
    set(TBB_URL "https://www.threadingbuildingblocks.org/sites/default/files/software_releases/windows/tbb44_20160413oss_win.zip")
elseif(PLATFORM_APPLE)
    set(TBB_URL "https://www.threadingbuildingblocks.org/sites/default/files/software_releases/mac/tbb44_20160413oss_osx.tgz")
endif()

if(PLATFORM_WINDOWS)
elseif(PLATFORM_LINUX)
    if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
        set(TBB_COMPILER_OPTION "compiler=clang")
    endif()

    if(RELEASE_CONFIG)
        set(TBB_BUILD_CONFIG "release")
        set(TBB_BUILD_TARGETS "tbb_release tbbmalloc_release")
    else()
        set(TBB_BUILD_CONFIG "debug"
        set(TBB_BUILD_TARGETS "tbb_debug tbbmalloc_debug")
    endif()

    ExternalProject_Add( tbb
            SOURCE_DIR ${TBB_SOURCE_DIR}
            URL "https://www.threadingbuildingblocks.org/sites/default/files/software_releases/source/tbb44_20160413oss_src.tgz"
            UPDATE_COMMAND ""
            PATCH_COMMAND patch ${CMAKE_SOURCE_DIR}/tbb/Makefile < ${CMAKE_SOURCE_DIR}/cmake/patches/tbb/Makefile.patch
            CONFIGURE_COMMAND ""
            BUILD_COMMAND make -j8 ${TBB_COMPILER_OPTION} arch=${ARCHITECTURE_PREFIX} tbb_build_prefix=tbb tbb_${TBB_BUILD_CONFIG} tbbmalloc_${TBB_BUILD_CONFIG}
            BUILD_IN_SOURCE 1
            INSTALL_COMMAND python cmake/scripts/tbb/install.py
            CMAKE_ARGS
            )
elseif(PLATFORM_APPLE)
endif()

#add_library(zlib UNKNOWN IMPORTED)
#set_property(TARGET zlib PROPERTY IMPORTED_LOCATION ${ZLIB_LIBRARY_DIR}/${ZLIB_NAME}.a)