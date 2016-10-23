cmake_minimum_required (VERSION 3.0)

include(cmake/platform_detection.cmake)

SET(ARCHIVE_DIRECTORY "lib")
SET(BINARY_DIRECTORY "bin")

if("${CMAKE_BUILD_TYPE}" STREQUAL "Release")
    set(RELEASE_CONFIG 1 )
else()
    set(RELEASE_CONFIG 0 )
endif()

project(CCGOnlineAll)

if(CMAKE_SIZEOF_VOID_P EQUAL 8)
    set(PLATFORM_SIZE 64)
    if(PLATFORM_WINDOWS OR PLATFORM_LINUX)
        set(ARCHITECTURE_PREFIX "intel64")
    endif()
    add_definitions(-DPLATFORM_SIZE=64)
    add_definitions(-DX64)
    set(WINDOWS_EXTERNAL_DLL_TYPE x64)
else()
    set(PLATFORM_SIZE 32)
    if(PLATFORM_WINDOWS OR PLATFORM_LINUX)
        set(ARCHITECTURE_PREFIX "ia32")
    endif()
    add_definitions(-DPLATFORM_SIZE=32)
    set(WINDOWS_EXTERNAL_DLL_TYPE x86)
endif()

set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

# misc config
if(CMAKE_CONFIGURATION_TYPES)
    if(RELEASE_CONFIG)
        set(CMAKE_CONFIGURATION_TYPES Release )
    else()
        set(CMAKE_CONFIGURATION_TYPES Debug )
    endif()
endif()

include(cmake/copy_macros.cmake)

# todo: convert to options
add_definitions(-DENABLE_LOGGING)
add_definitions(-DTRACK_MEMORY_ALLOCATIONS)

# directories
set(CCGONLINE_ROOT "${CMAKE_CURRENT_SOURCE_DIR}")
set(PROJECT_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/..")
set(EXTERNAL_PROJECT_SHARED_LIBRARY_DIR "${CMAKE_BINARY_DIR}/external/lib")

# external package setups
include(ExternalProject)
include(cmake/tbb.cmake)
include(cmake/loki.cmake)
include(cmake/gtest.cmake)

link_directories(${EXTERNAL_PROJECT_SHARED_LIBRARY_DIR})

include(cmake/compiler_options.cmake)

# default libraries to link in per-platform
if(PLATFORM_WINDOWS)
    set(PLATFORM_DEP_LIBS Shlwapi DbgHelp)
elseif(PLATFORM_LINUX OR PLATFORM_APPLE)
    set(PLATFORM_DEP_LIBS pthread)
endif()

if(NOT RELEASE_CONFIG)
    add_definitions(-D_DEBUG)
endif()

#targets
#add_subdirectory(IPCore)
add_subdirectory(pugixml)
#add_subdirectory(IPShared)
#add_subdirectory(IPDatabase)

#testing
#add_subdirectory(IPCoreTest)
#add_subdirectory(IPSharedTest)
#add_subdirectory(IPDatabaseTest)
