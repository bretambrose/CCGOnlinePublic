cmake_minimum_required(VERSION 2.8)
project(IPDatabaseTest)

file(GLOB IPDATABASETEST_HEADERS "${CMAKE_CURRENT_SOURCE_DIR}/include/IPDatabaseTest/*.h")
file(GLOB IPDATABASETEST_GENERATEDCODE_HEADERS "${CMAKE_CURRENT_SOURCE_DIR}/include/IPDatabaseTest/GeneratedCode/*.h")

file(GLOB IPDATABASETEST_SRC "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp")
file(GLOB IPDATABASETEST_GENERATEDCODE_SRC "${CMAKE_CURRENT_SOURCE_DIR}/src/GeneratedCode/*.cpp")

file(GLOB IPDATABASETEST_EXE_SOURCE
	${IPDATABASETEST_HEADERS}
	${IPDATABASETEST_GENERATEDCODE_HEADERS}
	${IPDATABASETEST_SRC}
	${IPDATABASETEST_GENERATEDCODE_SRC}
)

if(PLATFORM_WINDOWS)
	if(MSVC)
		source_group("include" FILES ${IPDATABASETEST_HEADERS})
		source_group("include\\GeneratedCode" FILES ${IPDATABASETEST_GENERATEDCODE_HEADERS})

		source_group("src" FILES ${IPDATABASETEST_SRC})
		source_group("src\\GeneratedCode" FILES ${IPDATABASETEST_GENERATEDCODE_SRC})
	endif()
endif()

include_directories(${LOKI_INCLUDES_PATH} ${TBB_INCLUDES_PATH} ${CCGONLINE_ROOT}/IPCore/include ${CCGONLINE_ROOT}/pugixml/include ${CCGONLINE_ROOT}/IPShared/include ${CCGONLINE_ROOT}/IPDatabase/include ${CCGONLINE_ROOT}/IPDatabaseTest/include)

add_executable(IPDatabaseTest ${IPDATABASETEST_EXE_SOURCE})

target_link_libraries(IPDatabaseTest IPDatabase IPShared IPCore odbc32 ${GTEST_LIBS} ${TBB_LIBS} ${PLATFORM_DEP_LIBS})

copyDlls(IPDatabaseTest IPDatabase IPShared IPCore gtest)
copyTbbDlls(IPDatabaseTest ${TBB_LIBS})
copyExternalDlls(IPDatabaseTest)
copyExeData(IPDatabaseTest)
copyDbSettings(IPDatabaseTest)


