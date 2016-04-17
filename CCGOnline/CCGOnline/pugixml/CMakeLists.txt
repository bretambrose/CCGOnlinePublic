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

if(PLATFORM_WINDOWS AND MSVC AND BUILD_SHARED_LIBS) 
#    add_definitions("-DIPCORE_EXPORTS=1")
endif()

add_library(pugixml ${LIBTYPE} ${PUGIXML_LIBRARY_SOURCE})

# todo: decide if install should set up binary directories or library form
install (TARGETS pugixml ARCHIVE DESTINATION ${ARCHIVE_DIRECTORY}/${PLATFORM_INSTALL_BINARY_PREFIX}/\${CMAKE_INSTALL_CONFIG_NAME}
                         LIBRARY DESTINATION lib/${PLATFORM_INSTALL_BINARY_PREFIX}/\${CMAKE_INSTALL_CONFIG_NAME}
				             RUNTIME DESTINATION bin/${PLATFORM_INSTALL_BINARY_PREFIX}/\${CMAKE_INSTALL_CONFIG_NAME})

install (FILES ${PUGIXML_HEADERS} DESTINATION include/pugixml)



