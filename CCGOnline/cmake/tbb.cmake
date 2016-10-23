
set(TBB_SOURCE_DIR "${CMAKE_BINARY_DIR}/tbb_src")

if(PLATFORM_WINDOWS)
    set(TBB_URL "https://www.threadingbuildingblocks.org/sites/default/files/software_releases/windows/tbb44_20160413oss_win.zip")
elseif(PLATFORM_LINUX)
    set(TBB_URL "https://www.threadingbuildingblocks.org/sites/default/files/software_releases/source/tbb2017_20160916oss_src.tgz")
elseif(PLATFORM_APPLE)
    set(TBB_URL "https://www.threadingbuildingblocks.org/sites/default/files/software_releases/mac/tbb44_20160413oss_osx.tgz")
endif()

if(RELEASE_CONFIG)
    set(TBB_BUILD_CONFIG "release")
    set(TBB_BUILD_TARGETS "tbb_release tbbmalloc_release")
    set(TBB_LIBS tbb tbbmalloc tbbmalloc_proxy)
else()
    set(TBB_BUILD_CONFIG "debug")
    set(TBB_BUILD_TARGETS "tbb_debug tbbmalloc_debug")
    set(TBB_LIBS tbb_debug tbbmalloc_debug tbbmalloc_proxy_debug)
endif()

if(PLATFORM_WINDOWS)
    # TODO: Add
elseif(PLATFORM_LINUX)
    if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
        set(TBB_COMPILER_OPTION "compiler=clang")
    endif()

    ExternalProject_Add( tbb_src
            SOURCE_DIR ${TBB_SOURCE_DIR}
            URL "${TBB_URL}"
            UPDATE_COMMAND ""
            PATCH_COMMAND patch ${TBB_SOURCE_DIR}/Makefile < ${CMAKE_SOURCE_DIR}/cmake/patches/tbb/Makefile.patch
            CONFIGURE_COMMAND ""
            BUILD_COMMAND make -j8 ${TBB_COMPILER_OPTION} arch=${ARCHITECTURE_PREFIX} tbb_build_prefix=tbb tbb_${TBB_BUILD_CONFIG} tbbmalloc_${TBB_BUILD_CONFIG}
            BUILD_IN_SOURCE 1
            INSTALL_COMMAND python ${CMAKE_SOURCE_DIR}/cmake/scripts/tbb/install.py --config ${TBB_BUILD_CONFIG} --dest ${EXTERNAL_PROJECT_SHARED_LIBRARY_DIR}
            CMAKE_ARGS
            )
elseif(PLATFORM_APPLE)
    # TODO: Add
endif()

set(TBB_INCLUDES_PATH "${TBB_SOURCE_DIR}/include")

foreach(TBB_LIB_NAME IN ITEMS ${TBB_LIBS})
    add_library(${TBB_LIB_NAME} UNKNOWN IMPORTED)
    set_property(TARGET ${TBB_LIB_NAME} PROPERTY IMPORTED_LOCATION ${EXTERNAL_PROJECT_SHARED_LIBRARY_DIR}/${PLATFORM_LIBRARY_PREFIX}${TBB_LIB_NAME}.${DYNAMIC_LIBRARY_SUFFIX})
endforeach()

if(NOT RELEASE_CONFIG)
    add_definitions(-DTBB_USE_DEBUG=1)
endif()

include_directories(${TBB_INCLUDES_PATH})
