cmake_minimum_required (VERSION 2.8)

SET(PLATFORM_WINDOWS 0)
SET(PLATFORM_LINUX 0)
SET(PLATFORM_APPLE 0)

if(WIN32)
	SET(PLATFORM_WINDOWS 1)
elseif(APPLE)
	SET(PLATFORM_APPLE 1)
elseif(UNIX)
	SET(PLATFORM_LINUX 1)
else()
	message(FATAL_ERROR "Unknown host OS; unable to determine platform compilation target")
endif()

if(PLATFORM_WINDOWS)
    message(STATUS "Generating windows build config")
    add_definitions("-DPLATFORM_WINDOWS")  # do not use in header files
    set(PLATFORM_INSTALL_BINARY_PREFIX "windows")
elseif(PLATFORM_LINUX)
    message(STATUS "Generating linux build config")
    add_definitions("-DPLATFORM_LINUX")  # do not use in header files
    set(PLATFORM_INSTALL_BINARY_PREFIX "linux")
elseif(PLATFORM_APPLE)
    message(STATUS "Generating Mac OSX build config")
    add_definitions("-DPLATFORM_APPLE")  # do not use in header files
    set(PLATFORM_INSTALL_BINARY_PREFIX "mac")
else()
    message(FATAL_ERROR "Unknown target platform.  How did this happen?")
endif()

SET(ARCHIVE_DIRECTORY "lib")

if("${CMAKE_BUILD_TYPE}" STREQUAL "Release")
    set(RELEASE_CONFIG 1 )
else()
    set(RELEASE_CONFIG 0 )
endif()

project(CCGOnlineAll)

# install syntax (after building): cmake -DCMAKE_INSTALL_CONFIG_NAME=<Release/Debug> -DCMAKE_INSTALL_PREFIX=<install_root> -P cmake_install.cmake
if(CMAKE_SIZEOF_VOID_P EQUAL 8)
    set(PLATFORM_SIZE 64)
    if(PLATFORM_WINDOWS OR PLATFORM_LINUX)
        set(ARCHITECTURE_PREFIX "intel64")
        set(PLATFORM_INSTALL_BINARY_PREFIX "${PLATFORM_INSTALL_BINARY_PREFIX}/${ARCHITECTURE_PREFIX}")
    endif()
    add_definitions(-DPLATFORM_SIZE=64)
    add_definitions(-DX64)
    set(WINDOWS_EXTERNAL_DLL_TYPE x64)
else()
    set(PLATFORM_SIZE 32)
    if(PLATFORM_WINDOWS OR PLATFORM_LINUX)
        set(ARCHITECTURE_PREFIX "ia32")
        set(PLATFORM_INSTALL_BINARY_PREFIX "${PLATFORM_INSTALL_BINARY_PREFIX}/${ARCHITECTURE_PREFIX}")
    endif()
    add_definitions(-DPLATFORM_SIZE=32)
    set(WINDOWS_EXTERNAL_DLL_TYPE x86)
endif()

message(STATUS "Install binary prefix: ${PLATFORM_INSTALL_BINARY_PREFIX}")

SET(LIBTYPE STATIC)
if(PLATFORM_WINDOWS)
    SET(INTERNAL_LIBRARY_SUFFIX "lib")
    SET(PLATFORM_LIBRARY_PREFIX "")
    SET(DYNAMIC_LIBRARY_SUFFIX "dll")
elseif(PLATFORM_LINUX OR PLATFORM_APPLE)
    SET(INTERNAL_LIBRARY_SUFFIX "a")
    SET(PLATFORM_LIBRARY_PREFIX "lib")
    SET(DYNAMIC_LIBRARY_SUFFIX "so")
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
set(EXTERNAL_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/external")
set(PROJECT_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/..")

# external package setups
include(ExternalProject)
include(cmake/tbb.cmake)
include(cmake/loki.cmake)
include(cmake/gtest.cmake)

# compiler options
if(PLATFORM_WINDOWS)
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /bigobj")
else()
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
endif()

# warning control
if(PLATFORM_WINDOWS)
    if(MSVC)
        # warnings as errors, max warning level (4)
        if(NOT CMAKE_CXX_FLAGS MATCHES "/WX")
            set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /WX")
        endif()

        # taken from http://stackoverflow.com/questions/2368811/how-to-set-warning-level-in-cmake
        if(CMAKE_CXX_FLAGS MATCHES "/W[0-4]")
            string(REGEX REPLACE "/W[0-4]" "/W4" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
        else()
            set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4")
        endif()
    endif()
elseif(PLATFORM_LINUX OR PLATFORM_APPLE)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Werror")
endif()

# default libraries to link in per-platform
if(PLATFORM_WINDOWS)
    set(PLATFORM_DEP_LIBS Shlwapi DbgHelp)
elseif(PLATFORM_LINUX OR PLATFORM_APPLE)
    set(PLATFORM_DEP_LIBS pthread)
endif()

if(NOT RELEASE_CONFIG)
    add_definitions(-D_DEBUG)
endif()

#set(GTEST_LIBS gtest gmock)

#targets
add_subdirectory(IPCore)
add_subdirectory(pugixml)
#add_subdirectory(IPShared)
#add_subdirectory(IPDatabase)

#testing
#add_subdirectory(IPCoreTest)
#add_subdirectory(IPSharedTest)
#add_subdirectory(IPDatabaseTest)
