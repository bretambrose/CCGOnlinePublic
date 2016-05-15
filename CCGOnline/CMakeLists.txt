cmake_minimum_required (VERSION 2.8)

SET(PLATFORM_WINDOWS 0)
SET(PLATFORM_LINUX 0)
SET(PLATFORM_APPLE 0)

if("${TARGET_ARCH}" STREQUAL "WINDOWS")
    SET(PLATFORM_WINDOWS 1)
elseif("${TARGET_ARCH}" STREQUAL "LINUX")
    SET(PLATFORM_LINUX 1)
elseif("${TARGET_ARCH}" STREQUAL "APPLE")
    SET(PLATFORM_APPLE 1)
else()
    message(STATUS "TARGET_ARCH not specified; inferring host OS to be platform compilation target")
    if(WIN32)
        SET(PLATFORM_WINDOWS 1)
    elseif(APPLE)
        SET(PLATFORM_APPLE 1)
    elseif(UNIX)
        SET(PLATFORM_LINUX 1)
    else()
        message(FATAL_ERROR "Unknown host OS; unable to determine platform compilation target")
    endif()
endif()

if(PLATFORM_WINDOWS)
    message(STATUS "Generating windows build config")
    add_definitions("-DPLATFORM_WINDOWS")
    set(PLATFORM_INSTALL_BINARY_PREFIX "windows")
elseif(PLATFORM_LINUX)
    message(STATUS "Generating linux build config")
    add_definitions("-DPLATFORM_LINUX")
    set(PLATFORM_INSTALL_BINARY_PREFIX "linux")
elseif(PLATFORM_APPLE)
    message(STATUS "Generating Mac OSX build config")
    add_definitions("-DPLATFORM_APPLE")
    set(PLATFORM_INSTALL_BINARY_PREFIX "mac")
else()
    message(FATAL_ERROR "Unknown target platform.  How did this happen?")
endif()

if("${DYNAMIC_LINKING}" STREQUAL "1")
    SET(BUILD_SHARED_LIBS 1)
    SET(ARCHIVE_DIRECTORY "bin")
    message(STATUS "Dynamic linking enabled")
else()
    SET(BUILD_SHARED_LIBS 0)
    SET(ARCHIVE_DIRECTORY "lib")
    message(STATUS "Dynamic linking disabled")
endif()

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
		set(PLATFORM_INSTALL_BINARY_PREFIX "${PLATFORM_INSTALL_BINARY_PREFIX}/intel64")
	endif()
	add_definitions(-DPLATFORM_SIZE=64)
	add_definitions(-DX64)
	set(WINDOWS_EXTERNAL_DLL_TYPE x64)
else()
	set(PLATFORM_SIZE 32)
	if(PLATFORM_WINDOWS OR PLATFORM_LINUX)
		set(PLATFORM_INSTALL_BINARY_PREFIX "${PLATFORM_INSTALL_BINARY_PREFIX}/ia32")
	endif()
	add_definitions(-DPLATFORM_SIZE=32)
	set(WINDOWS_EXTERNAL_DLL_TYPE x86)
endif()

message(STATUS "Install binary prefix: ${PLATFORM_INSTALL_BINARY_PREFIX}")

if(BUILD_SHARED_LIBS)
	SET(LIBTYPE SHARED)
	if(PLATFORM_WINDOWS)
		add_definitions("-DUSE_IMPORT_EXPORT")
		SET(SUFFIX dll)
	elseif(PLATFORM_LINUX)
		SET(SUFFIX so)
	elseif(PLATFORM_APPLE)
		SET(SUFFIX dylib)
	endif()
else()
	SET(LIBTYPE STATIC)
	if(PLATFORM_WINDOWS)
		SET(SUFFIX lib)
	elseif(PLATFORM_LINUX OR PLATFORM_ANDROID OR PLATFORM_APPLE)
		SET(SUFFIX a)
	endif()
	
	set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
endif()


macro(copyDlls exeName)
	if(PLATFORM_WINDOWS AND BUILD_SHARED_LIBS)
		foreach(arg ${ARGN})
			add_custom_command(TARGET ${exeName}
									 POST_BUILD
									 COMMAND ${CMAKE_COMMAND} -E copy_if_different
									 "${CMAKE_BINARY_DIR}/${arg}/$<CONFIGURATION>/${arg}.dll"      
									 ${CMAKE_CURRENT_BINARY_DIR}/$<CONFIGURATION>/)    
		endforeach()
	endif()
endmacro()

# misc config
if(CMAKE_CONFIGURATION_TYPES)
	if(RELEASE_CONFIG)
		set(CMAKE_CONFIGURATION_TYPES Release )
	else()
		set(CMAKE_CONFIGURATION_TYPES Debug )
	endif()
endif()

# todo: convert to options
add_definitions(-DENABLE_LOGGING)
add_definitions(-DTRACK_MEMORY_ALLOCATIONS)

# directories
set(CCGONLINE_ROOT "${CMAKE_CURRENT_SOURCE_DIR}")
set(EXTERNAL_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/External")
set(PROJECT_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/..")

# external package setups

# loki
set(LOKI_INCLUDES_PATH ${EXTERNAL_ROOT}/loki/include)

# tbb
# todo: remove hard-coded visual studio part of path
set(TBB_INCLUDES_PATH ${EXTERNAL_ROOT}/tbb/include)

if(PLATFORM_SIZE EQUAL 64)
	set(TBB_EXPORT_LINK_PATH ${EXTERNAL_ROOT}/tbb/lib/intel64/vc12)
	set(TBB_BINARY_PATH ${EXTERNAL_ROOT}/tbb/bin/intel64/vc12)
else()
	set(TBB_EXPORT_LINK_PATH ${EXTERNAL_ROOT}/tbb/lib/ia32/vc12)
	set(TBB_BINARY_PATH ${EXTERNAL_ROOT}/tbb/bin/ia32/vc12)
endif()
link_directories(${TBB_EXPORT_LINK_PATH})

if(RELEASE_CONFIG)
	set(TBB_LIBS tbb tbb_preview tbbmalloc tbbmalloc_proxy)
else()
	set(TBB_LIBS tbb_debug tbb_preview_debug tbbmalloc_debug tbbmalloc_proxy_debug)
	add_definitions(-DTBB_USE_DEBUG=1)
endif()

macro(copyTbbDlls exeName)
	if(PLATFORM_WINDOWS)
		foreach(arg ${ARGN})
			add_custom_command(TARGET ${exeName}
									 POST_BUILD
									 COMMAND ${CMAKE_COMMAND} -E copy_if_different
									 "${TBB_BINARY_PATH}/${arg}.dll"      
									 ${CMAKE_CURRENT_BINARY_DIR}/$<CONFIGURATION>/)    
		endforeach()
	endif()
endmacro()

macro(copyExternalDlls exeName)
	if(PLATFORM_WINDOWS)
		set(EXTERNAL_DLL_DIR ${EXTERNAL_ROOT}/DLLs/${WINDOWS_EXTERNAL_DLL_TYPE})
		file(GLOB EXTERNAL_DLLS ${EXTERNAL_DLL_DIR}/*.dll)
		foreach(file ${EXTERNAL_DLLS})
			add_custom_command(TARGET ${exeName}
									 POST_BUILD
									 COMMAND ${CMAKE_COMMAND} -E copy_if_different
									 "${file}"      
									 ${CMAKE_CURRENT_BINARY_DIR}/$<CONFIGURATION>/)    
		endforeach()
	endif()
endmacro()

macro(copyExeData projectName)
	set(EXE_DATA_DIR ${PROJECT_ROOT}/Run/Data/${projectName})
	add_custom_command(TARGET ${projectName} POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_directory ${EXE_DATA_DIR} ${CMAKE_CURRENT_BINARY_DIR}/)
endmacro()

macro(copyDbSettings projectName)
	set(DB_SETTINGS_FILE ${PROJECT_ROOT}/Settings/SQLServer/DBSettings.txt)
	add_custom_command(TARGET ${projectName} POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_if_different ${DB_SETTINGS_FILE} ${CMAKE_CURRENT_BINARY_DIR}/Settings/)
endmacro()

# compiler options
if(PLATFORM_WINDOWS)
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /bigobj")
else()
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-exceptions")
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

#release pdbs in windows
if(PLATFORM_WINDOWS AND BUILD_SHARED_LIBS)
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /Zi")
    set(CMAKE_SHARED_LINKER_FLAGS_RELEASE "${CMAKE_SHARED_LINKER_FLAGS_RELEASE} /DEBUG /OPT:REF /OPT:ICF")
endif()

# default libraries to link in per-platform
if(PLATFORM_WINDOWS)
    set(PLATFORM_DEP_LIBS Shlwapi DbgHelp)
elseif(PLATFORM_LINUX OR PLATFORM_APPLE)
    set(PLATFORM_DEP_LIBS pthread)
endif()

set(GTEST_LIBS gtest gmock)

#targets
add_subdirectory(IPCore)
add_subdirectory(pugixml)
add_subdirectory(IPShared)
add_subdirectory(IPDatabase)

#testing
add_subdirectory(googletest)
add_subdirectory(IPCoreTest)
add_subdirectory(IPSharedTest)
add_subdirectory(IPDatabaseTest)
