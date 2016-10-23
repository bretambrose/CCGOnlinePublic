
set(GTEST_SOURCE_DIR "${CMAKE_BINARY_DIR}/gtest")

set(GTEST_URL "https://github.com/google/googletest.git")


if(PLATFORM_WINDOWS)
    # TODO: add
elseif(PLATFORM_LINUX)

    ExternalProject_Add( gtest
            SOURCE_DIR ${GTEST_SOURCE_DIR}
            GIT_REPOSITORY "${GTEST_URL}"
            GIT_TAG c99458533a9b4c743ed51537e25989ea55944908
            INSTALL_COMMAND ""
            CMAKE_ARGS
                -Dgtest_force_shared_crt=ON
            )
elseif(PLATFORM_APPLE)
    # TODO: add
endif()

set(GTEST_LIBS gtest gmock)
