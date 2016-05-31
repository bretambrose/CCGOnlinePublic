cmake_minimum_required(VERSION 2.8)
project(pugixml)


file(GLOB PUGIXML_HEADERS "${CMAKE_CURRENT_SOURCE_DIR}/include/pugixml/*.h")
file(GLOB PUGIXML_SOURCE "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp")

file(GLOB PUGIXML_LIBRARY_SOURCE
	${PUGIXML_HEADERS}
	${PUGIXML_SOURCE}
) 

if(MSVC)
	source_group("include\\pugixml" FILES ${PUGIXML_HEADERS})
	source_group("src" FILES ${PUGIXML_SOURCE})
endif(MSVC)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

add_library(pugixml ${LIBTYPE} ${PUGIXML_LIBRARY_SOURCE})





