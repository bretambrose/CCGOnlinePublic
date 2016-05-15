cmake_minimum_required(VERSION 2.8)
project(IPSharedTest)

file(GLOB IPSHAREDTEST_HEADERS "${CMAKE_CURRENT_SOURCE_DIR}/include/IPSharedTest/*.h")
file(GLOB IPSHAREDTEST_HELPERS_HEADERS "${CMAKE_CURRENT_SOURCE_DIR}/include/IPSharedTest/Helpers/*.h")
file(GLOB IPSHAREDTEST_GENERATEDCODE_HEADERS "${CMAKE_CURRENT_SOURCE_DIR}/include/IPSharedTest/GeneratedCode/*.h")

file(GLOB IPSHAREDTEST_SRC "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp")
file(GLOB IPSHAREDTEST_HELPERS_SRC "${CMAKE_CURRENT_SOURCE_DIR}/src/Helpers/*.cpp")
file(GLOB IPSHAREDTEST_GENERATEDCODE_SRC "${CMAKE_CURRENT_SOURCE_DIR}/src/GeneratedCode/*.cpp")

file(GLOB IPSHAREDTEST_EXE_SOURCE
	${IPSHAREDTEST_HEADERS}
	${IPSHAREDTEST_HELPERS_HEADERS}
	${IPSHAREDTEST_GENERATEDCODE_HEADERS}
	${IPSHAREDTEST_SRC}
	${IPSHAREDTEST_HELPERS_SRC}
	${IPSHAREDTEST_GENERATEDCODE_SRC}
)

if(PLATFORM_WINDOWS)
	if(MSVC)
		source_group("include" FILES ${IPSHAREDTEST_HEADERS})
		source_group("include\\Helpers" FILES ${IPSHAREDTEST_HELPERS_HEADERS})
		source_group("include\\GeneratedCode" FILES ${IPSHAREDTEST_GENERATEDCODE_HEADERS})

		source_group("src" FILES ${IPSHAREDTEST_SRC})
		source_group("src\\Helpers" FILES ${IPSHAREDTEST_HELPERS_SRC})
		source_group("src\\GeneratedCode" FILES ${IPSHAREDTEST_GENERATEDCODE_SRC})
	endif()
endif()

include_directories(${LOKI_INCLUDES_PATH} ${TBB_INCLUDES_PATH} ${CCGONLINE_ROOT}/IPCore/include ${CCGONLINE_ROOT}/pugixml/include ${CCGONLINE_ROOT}/IPShared/include ${CCGONLINE_ROOT}/IPSharedTest/include)

add_executable(IPSharedTest ${IPSHAREDTEST_EXE_SOURCE})

target_link_libraries(IPSharedTest IPShared IPCore ${GTEST_LIBS} ${TBB_LIBS} ${PLATFORM_DEP_LIBS})

copyDlls(IPSharedTest IPShared IPCore gtest)
copyTbbDlls(IPSharedTest ${TBB_LIBS})
copyExternalDlls(IPSharedTest)
copyExeData(IPSharedTest)

