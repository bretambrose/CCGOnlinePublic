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

set(WINDOWS_DLL_EXPORT "__declspec(dllexport)")
set(WINDOWS_DLL_IMPORT "__declspec(dllimport)")

if(PLATFORM_WINDOWS)
    add_definitions("-DPUGIXML_API=${WINDOWS_DLL_EXPORT}")
    add_definitions("-DPUGIXML_CLASS=${WINDOWS_DLL_IMPORT}")
endif()

add_library(pugixml SHARED ${PUGIXML_LIBRARY_SOURCE})

if(PLATFORM_WINDOWS)
    target_compile_definitions(pugixml PUBLIC -DPUGIXML_API=${WINDOWS_DLL_EXPORT})
endif()



