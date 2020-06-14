macro (generate_git_version _out_var)
    find_package (Git QUIET)
    if (GIT_FOUND)
        execute_process (
            COMMAND ${GIT_EXECUTABLE} describe --abbrev=6 --always --tags
            OUTPUT_VARIABLE ${_out_var}
            OUTPUT_STRIP_TRAILING_WHITESPACE
            ERROR_QUIET
            WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        )
        if (${_out_var} STREQUAL "")
            set (${_out_var} "unknown")
        endif ()
    endif ()
endmacro ()

macro (define_dll name)
    # set(options OPTIONAL FAST)
    set(oneValueArgs FOLDER)
    set(multiValueArgs LINK DELAY_LINK)
    cmake_parse_arguments(
        ARG 
        "${options}" 
        "${oneValueArgs}"
        "${multiValueArgs}" 
        ${ARGN}
    )
    file (
        GLOB_RECURSE srcs
        CONFIGURE_DEPENDS *.cpp
    )
    add_library (${name} SHARED ${srcs})
    target_compile_definitions (
        ${name}
        PRIVATE DLLNAME=${name}
    )
    target_link_libraries (
        ${name}
        ${ARG_LINK}
        ${ARG_DELAY_LINK}
    )
    install (
        TARGETS ${name}
        RUNTIME DESTINATION .
        ARCHIVE DESTINATION Lib
    )
    install_pdb (${name})
    set_target_properties (
        ${name}
        PROPERTIES FOLDER ${ARG_FOLDER}
    )
    if (ARG_DELAY_LINK)
        target_link_libraries (${name} delayimp)
        foreach (target ${ARG_DELAY_LINK})
            target_link_options (${name} PRIVATE /DELAYLOAD:${target}.dll)
        endforeach ()
    endif ()
endmacro ()

macro (define_exe name)
    set(options TEMP)
    set(oneValueArgs FOLDER)
    set(multiValueArgs LINK DELAY_LINK)
    cmake_parse_arguments(
        ARG 
        "${options}" 
        "${oneValueArgs}"
        "${multiValueArgs}" 
        ${ARGN}
    )
    set (IS_TEMP $<BOOL:${ARG_TEMP}>)
    file (
        GLOB_RECURSE srcs
        CONFIGURE_DEPENDS *.cpp
    )
    add_executable (${name} ${srcs})
    target_compile_definitions (
        ${name}
        PRIVATE EXENAME=${name}
    )
    target_link_libraries (
        ${name}
        ${ARG_LINK}
        ${ARG_DELAY_LINK}
    )
    install (TARGETS ${name}
        RUNTIME DESTINATION $<IF:${IS_TEMP},Temp,.>
    )
    set_target_properties (
        ${name}
        PROPERTIES FOLDER "$<IF:${IS_TEMP},Temp,.>/${ARG_FOLDER}"
    )
    if (ARG_DELAY_LINK)
        target_link_libraries (${name} delayimp)
        foreach (target ${ARG_DELAY_LINK})
            target_link_options (${name} PRIVATE /DELAYLOAD:${target}.dll)
        endforeach ()
    endif ()
endmacro ()

function (install_dependency arg)
    list (LENGTH ARGV argv_len)
    set (i 0)
    while( i LESS ${argv_len})
        list(GET ARGV ${i} argv_value)
        message(STATUS "Processing dependency: ${argv_value} (${VCPKG_TARGET_TRIPLET})...")
        execute_process(
            COMMAND ${VCPKG_EXECUTABLE} install ${argv_value} --triplet ${VCPKG_TARGET_TRIPLET} 
            WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
            RESULT_VARIABLE VCPKG_INSTALL_RES
            OUTPUT_QUIET
        )
        if(NOT (${VCPKG_INSTALL_RES} EQUAL 0))
            message(SEND_ERROR "Errors occurred when processing dependency ${argv_value} (${VCPKG_TARGET_TRIPLET}).")
        else()
            message(STATUS "succeeded.")
        endif()
        math(EXPR i "${i} + 1")
    endwhile()
endfunction()
