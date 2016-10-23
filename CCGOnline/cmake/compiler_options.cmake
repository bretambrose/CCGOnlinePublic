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
