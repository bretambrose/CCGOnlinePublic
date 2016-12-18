
set(GTEST_SOURCE_DIR "${CMAKE_BINARY_DIR}/gtest_src")

set(GTEST_URL "https://github.com/google/googletest.git")


if(PLATFORM_WINDOWS)
    # TODO: add
elseif(PLATFORM_LINUX)

    ExternalProject_Add( gtest_src
            SOURCE_DIR ${GTEST_SOURCE_DIR}
            GIT_REPOSITORY "${GTEST_URL}"
            GIT_TAG c99458533a9b4c743ed51537e25989ea55944908
            INSTALL_COMMAND python ${CMAKE_SOURCE_DIR}/cmake/gtest/install.py --dest ${EXTERNAL_PROJECT_DIR}/gtest --source ${CMAKE_BINARY_DIR}/gtest_src
            CMAKE_ARGS
                -Dgtest_force_shared_crt=ON
            )
elseif(PLATFORM_APPLE)
    # TODO: add
endif()

set(GTEST_LIBRARY_DIR "${EXTERNAL_PROJECT_DIR/gtest/lib}")
set(GTEST_INCLUDES_PATH "${EXTERNAL_PROJECT_DIR}/gtest/include")

add_library(gtest UNKNOWN IMPORTED)
set_property(TARGET gtest PROPERTY IMPORTED_LOCATION ${GTEST_LIBRARY_DIR}/${PLATFORM_LIBRARY_PREFIX}gtest.${STATIC_LIBRARY_SUFFIX})

include_directories(${GTEST_INCLUDES_PATH})
