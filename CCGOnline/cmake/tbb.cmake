
set(TBB_SOURCE_DIR "${CMAKE_SOURCE_DIR}/tbb")

if(PLATFORM_WINDOWS)
    set(TBB_URL "https://www.threadingbuildingblocks.org/sites/default/files/software_releases/windows/tbb44_20160413oss_win.zip")
elseif(PLATFORM_LINUX)
    set(TBB_URL "https://www.threadingbuildingblocks.org/sites/default/files/software_releases/source/tbb44_20160413oss_src.tgz")
    set(TBB_PATCH "patch Makefile < ${CMAKE_SOURCE_DIR}/cmake/patches/tbb/Makefile.patch")
        
    if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
        set(TBB_COMPILER_OPTION "compiler=clang")
    endif()

    if(RELEASE_CONFIG)
        set(TBB_BUILD_TARGETS "tbb_release tbbmalloc_release")
    else()
        set(TBB_BUILD_TARGETS "tbb_debug tbbmalloc_debug")
    endif()

    set(TBB_BUILD_COMMAND "make -j8 ${TBB_COMPILER_OPTION} arch=${ARCHITECTURE_PREFIX} tbb_build_prefix=tbb ${TBB_BUILD_TARGETS}")
elseif(PLATFORM_APPLE)
    set(TBB_URL "https://www.threadingbuildingblocks.org/sites/default/files/software_releases/mac/tbb44_20160413oss_osx.tgz")
endif()

ExternalProject_Add( tbb
        SOURCE_DIR ${TBB_SOURCE_DIR}
        URL ${TBB_URL}
        UPDATE_COMMAND ""
        PATCH_COMMAND "${TBB_PATCH}"
        CONFIGURE_COMMAND ""
        BUILD_COMMAND "${TBB_BUILD_COMMAND}"
        INSTALL_COMMAND ""
        CMAKE_ARGS
        )


#add_library(zlib UNKNOWN IMPORTED)
#set_property(TARGET zlib PROPERTY IMPORTED_LOCATION ${ZLIB_LIBRARY_DIR}/${ZLIB_NAME}.a)