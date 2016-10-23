
set(LOKI_SOURCE_DIR "${CMAKE_BINARY_DIR}/loki_src")

set(LOKI_VERSION "loki-0.1.7")
set(LOKI_URL "http://sourceforge.net/projects/loki-lib/files/Loki/Loki%200.1.7/${LOKI_VERSION}.zip")

if(PLATFORM_WINDOWS)
    # TODO: add
elseif(PLATFORM_LINUX)

    ExternalProject_Add( loki_src
        SOURCE_DIR ${LOKI_SOURCE_DIR}
        URL "${LOKI_URL}"
        DOWNLOAD_COMMAND wget ${LOKI_URL} && unzip ${LOKI_VERSION}.zip && mv ${LOKI_VERSION} loki_src && mv loki_src ${CMAKE_BINARY_DIR}
        UPDATE_COMMAND ""
        PATCH_COMMAND ""
        CONFIGURE_COMMAND ""
        BUILD_COMMAND make -j8 build-static
        BUILD_IN_SOURCE 1
        INSTALL_COMMAND ""
        CMAKE_ARGS
        )
elseif(PLATFORM_APPLE)
    # TODO: add
endif()

set(LOKI_LIBRARY_DIR "${CMAKE_CURRENT_BINARY_DIR}")
set(LOKI_INCLUDES_PATH "${LOKI_SOURCE_DIR}/include")

add_library(loki UNKNOWN IMPORTED)
set_property(TARGET loki PROPERTY IMPORTED_LOCATION ${CMAKE_CURRENT_BINARY_DIR}/${PLATFORM_LIBRARY_PREFIX}loki.${STATIC_LIBRARY_SUFFIX})

include_directories(${LOKI_INCLUDES_PATH})

