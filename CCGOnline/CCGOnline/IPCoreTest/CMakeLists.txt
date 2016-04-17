cmake_minimum_required(VERSION 2.8)
project(IPCoreTest)

file(GLOB IPCORE_TEST_SRC "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp")

if(PLATFORM_WINDOWS)
  if(MSVC)
    source_group("src" FILES ${IPCORE_TEST_SRC})
  endif()
endif()

include_directories(${LOKI_INCLUDES_PATH} ${TBB_INCLUDES_PATH} ${CCGONLINE_ROOT}/IPCore/include)

add_executable(IPCoreTest ${IPCORE_TEST_SRC})

target_link_libraries(IPCoreTest IPCore ${GTEST_LIBS} ${TBB_LIBS} ${PLATFORM_DEP_LIBS})

copyDlls(IPCoreTest IPCore gtest)
copyTbbDlls(IPCoreTest ${TBB_LIBS})
copyExeData(IPCoreTest)


