
set(GTEST_SOURCE_DIR "${CMAKE_SOURCE_DIR}/gtest_src")

set(GTEST_URL "https://github.com/google/googletest.git")

#if(RELEASE_CONFIG)
#    set(LOKI_BUILD_CONFIG "release")
#    set(LOKI_BUILD_TARGETS "tbb_release tbbmalloc_release")
#    set(LOKI_LIBRARY_SUFFIX "")
#else()
#    set(LOKI_BUILD_CONFIG "debug")
#    set(LOKI_BUILD_TARGETS "tbb_debug tbbmalloc_debug")
#    set(LOKI_LIBRARY_SUFFIX "_debug")
#endif()

if(PLATFORM_WINDOWS)
elseif(PLATFORM_LINUX)

    ExternalProject_Add( gtest_src
            SOURCE_DIR ${GTEST_SOURCE_DIR}
            GIT_REPOSITORY "${GTEST_URL}"
            GIT_TAG c99458533a9b4c743ed51537e25989ea55944908
            INSTALL_COMMAND "cmake "
            CMAKE_ARGS
                -Dgtest_force_shared_crt=ON
            )
elseif(PLATFORM_APPLE)
endif()

#set(TBB_LIBRARY_DIR "${CMAKE_SOURCE_DIR}/external/tbb/lib")

#foreach(TBB_LIB_NAME IN ITEMS "tbb" "tbbmalloc" "tbbmalloc_proxy")
#    add_library(${TBB_LIB_NAME}${TBB_LIBRARY_SUFFIX} UNKNOWN IMPORTED)
#    set_property(TARGET ${TBB_LIB_NAME}${TBB_LIBRARY_SUFFIX} PROPERTY IMPORTED_LOCATION ${TBB_LIBRARY_DIR}/lib${TBB_LIB_NAME}${TBB_LIBRARY_SUFFIX}.so)
#endforeach()

#set(GTEST_INCLUDES_PATH "${CMAKE_SOURCE_DIR}/external/gtest/include")

#link_directories(${TBB_LIBRARY_DIR})
#include_directories(${TBB_INCLUDES_PATH})

#if(RELEASE_CONFIG)
#    set(TBB_LIBS tbb tbbmalloc tbbmalloc_proxy)
#else()
#    set(TBB_LIBS tbb_debug tbbmalloc_debug tbbmalloc_proxy_debug)
#    add_definitions(-DTBB_USE_DEBUG=1)
#endif()