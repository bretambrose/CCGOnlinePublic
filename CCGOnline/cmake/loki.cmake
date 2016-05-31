
set(LOKI_SOURCE_DIR "${CMAKE_SOURCE_DIR}/loki_src")

set(LOKI_URL "http://sourceforge.net/projects/loki-lib/files/Loki/Loki%200.1.7/loki-0.1.7.zip")


if(PLATFORM_WINDOWS)
elseif(PLATFORM_LINUX)

    ExternalProject_Add( loki_src
            SOURCE_DIR ${LOKI_SOURCE_DIR}
            URL "${LOKI_URL}"
            UPDATE_COMMAND ""
            PATCH_COMMAND ""
            CONFIGURE_COMMAND ""
            BUILD_COMMAND make -j8 build-static
            BUILD_IN_SOURCE 1
            INSTALL_COMMAND python ${CMAKE_SOURCE_DIR}/cmake/scripts/loki/install.py
            CMAKE_ARGS
            )
elseif(PLATFORM_APPLE)
endif()

set(LOKI_LIBRARY_DIR "${CMAKE_SOURCE_DIR}/external/loki/lib")

add_library(loki UNKNOWN IMPORTED)
set_property(TARGET loki PROPERTY IMPORTED_LOCATION ${LOKI_LIBRARY_DIR}/${PLATFORM_LIBRARY_PREFIX}loki.${INTERNAL_LIBRARY_SUFFIX})

set(LOKI_INCLUDES_PATH "${CMAKE_SOURCE_DIR}/external/loki/include")

link_directories(${LOKI_LIBRARY_DIR})
include_directories(${LOKI_INCLUDES_PATH})

