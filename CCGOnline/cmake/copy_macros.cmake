# misc macros for copying loose artifacts
# still unsure of what "install" should mean at this point, so do things manually


macro(copyTbbDlls exeName)
    if(PLATFORM_WINDOWS)
        foreach(arg ${ARGN})
            add_custom_command(TARGET ${exeName}
                                     POST_BUILD
                                     COMMAND ${CMAKE_COMMAND} -E copy_if_different
                                     "${TBB_BINARY_PATH}/${arg}.dll"
                                     ${CMAKE_CURRENT_BINARY_DIR}/$<CONFIGURATION>/)
        endforeach()
    endif()
endmacro()

macro(copyExternalDlls exeName)
    if(PLATFORM_WINDOWS)
        set(EXTERNAL_DLL_DIR ${EXTERNAL_ROOT}/DLLs/${WINDOWS_EXTERNAL_DLL_TYPE})
        file(GLOB EXTERNAL_DLLS ${EXTERNAL_DLL_DIR}/*.dll)
        foreach(file ${EXTERNAL_DLLS})
            add_custom_command(TARGET ${exeName}
                                     POST_BUILD
                                     COMMAND ${CMAKE_COMMAND} -E copy_if_different
                                     "${file}"
                                     ${CMAKE_CURRENT_BINARY_DIR}/$<CONFIGURATION>/)
        endforeach()
    endif()
endmacro()

macro(copyExeData projectName)
    set(EXE_DATA_DIR ${PROJECT_ROOT}/Run/Data/${projectName})
    add_custom_command(TARGET ${projectName} POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_directory ${EXE_DATA_DIR} ${CMAKE_CURRENT_BINARY_DIR}/)
endmacro()

macro(copyDbSettings projectName)
    set(DB_SETTINGS_FILE ${PROJECT_ROOT}/Settings/SQLServer/DBSettings.txt)
    add_custom_command(TARGET ${projectName} POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_if_different ${DB_SETTINGS_FILE} ${CMAKE_CURRENT_BINARY_DIR}/Settings/)
endmacro()

