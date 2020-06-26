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

function (define_dll name)
    # set(options OPTIONAL FAST)
    set(oneValueArgs DESTINATION COMPONENT)
    set(multiValueArgs LINK DELAY_LINK INCLUDE RUNTIME DEFINE DEPENDENCY)
    cmake_parse_arguments(
        ARG 
        "${options}" 
        "${oneValueArgs}"
        "${multiValueArgs}" 
        ${ARGN}
    )
    if (NOT DEFINED ARG_COMPONENT)
        set(ARG_COMPONENT ${name})
    endif()
    file (
        GLOB_RECURSE srcs
        CONFIGURE_DEPENDS *.cpp
    )
    add_library (${name} SHARED ${srcs})
    if (DEFINED ARG_DEPENDENCY)
        add_dependencies (${name}
            ${ARG_DEPENDENCY}
	)
    endif ()
    if (DEFINED ARG_DEFINE)
        target_compile_definitions (
            ${name}
            PRIVATE 
            ${ARG_DEFINE}
        )
    endif ()
    if (DEFINED ARG_INCLUDE)
        target_include_directories (
            ${name}
            PRIVATE
            ${ARG_INCLUDE}
	    )
    endif ()
    if ((DEFINED ARG_LINK) OR (DEFINED ARG_DELAY_LINK))
        target_link_libraries (
            ${name}
            PRIVATE
            ${ARG_LINK}
            ${ARG_DELAY_LINK}
        )
    endif ()
    install (
        TARGETS ${name}
        RUNTIME DESTINATION ./${ARG_DESTINATION}
#        ARCHIVE DESTINATION ./${ARG_DESTINATION}/Lib
        COMPONENT ${ARG_COMPONENT}
    )
    if (DEFINED ARG_RUNTIME)
        add_custom_command(
            TARGET ${name}
            POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                    ${ARG_RUNTIME}     
                    $<TARGET_FILE_DIR:${name}>
        )
        install(
            FILES ${ARG_RUNTIME}
            DESTINATION ./${ARG_DESTINATION}
            COMPONENT ${ARG_COMPONENT}
		)
    endif ()
    if (ARG_DELAY_LINK)
#        target_link_libraries (${name} delayimp)
        foreach (target ${ARG_DELAY_LINK})
            target_link_options (${name} PRIVATE /DELAYLOAD:${target}.dll)
        endforeach ()
    endif ()
endfunction ()

function (define_exe name)
#    set(options TEMP)
    set(oneValueArgs DESTINATION)
    set(multiValueArgs LINK DELAY_LINK INCLUDE RUNTIME DEFINE DEPENDENCY)
    cmake_parse_arguments(
        ARG 
        "${options}" 
        "${oneValueArgs}"
        "${multiValueArgs}" 
        ${ARGN}
    )
    if (NOT DEFINED ARG_COMPONENT)
        set(ARG_COMPONENT ${name})
    endif()
    file (
        GLOB_RECURSE srcs
        CONFIGURE_DEPENDS *.cpp
    )
    add_executable (${name} ${srcs})
    if (DEFINED ARG_DEPENDENCY)
        add_dependencies (${name}
            ${ARG_DEPENDENCY}
	)
    endif ()
    if (DEFINED ARG_DEFINE)
        target_compile_definitions (
            ${name}
            PRIVATE 
            ${ARG_DEFINE}
        )
    endif ()
    if (DEFINED ARG_INCLUDE)
        target_include_directories (
            ${name}
            PRIVATE
            ${ARG_INCLUDE}
	    )
    endif ()
    if ((DEFINED ARG_LINK) OR (DEFINED ARG_DELAY_LINK))
        target_link_libraries (
            ${name}
            PRIVATE
            ${ARG_LINK}
            ${ARG_DELAY_LINK}
        )
    endif ()
    install (TARGETS ${name}
        RUNTIME DESTINATION ./${ARG_DESTINATION}
        COMPONENT ${ARG_COMPONENT}
    )
    if (DEFINED ARG_RUNTIME)
        add_custom_command(
            TARGET ${name}
            POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                    ${ARG_RUNTIME}     
                    $<TARGET_FILE_DIR:${name}>
        )
        install(
            FILES ${ARG_RUNTIME}
            DESTINATION ./${ARG_DESTINATION}
            COMPONENT ${ARG_COMPONENT}
		)
    endif ()
    if (ARG_DELAY_LINK)
#        target_link_libraries (${name} delayimp)
        foreach (target ${ARG_DELAY_LINK})
            target_link_options (${name} PRIVATE /DELAYLOAD:${target}.dll)
        endforeach ()
    endif ()
endfunction ()

function(def_py name)
    set(oneValueArgs DESTINATION)
    set(multiValueArgs DEPENDENCY RUNTIME)

    cmake_parse_arguments(
            ARG
            "${options}"
            "${oneValueArgs}"
            "${multiValueArgs}"
            ${ARGN}
    )
    add_custom_target(${name})
    if (DEFINED ARG_DEPENDENCY)
        add_dependencies(
                ${name}
                ${ARG_DEPENDENCY}
        )
    endif ()
#    add_custom_command(
#            TARGET ${name}
#            PRE_BUILD
#            COMMAND ${CMAKE_COMMAND} -E
#            copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/${name} ${ARG_DESTINATION}
#    )
    install(
            DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/${name}
            DESTINATION ${ARG_DESTINATION}
    )
    if (DEFINED ARG_RUNTIME)
        #        add_custom_command(
        #                TARGET ${name}
        #                POST_BUILD
        #                COMMAND ${CMAKE_COMMAND} -E copy_if_different
        #                ${ARG_RUNTIME}
        #                $<TARGET_FILE_DIR:${name}>
        #        )
        install(
                FILES ${ARG_RUNTIME}
                DESTINATION ${ARG_DESTINATION}/${name}
                COMPONENT ${ARG_COMPONENT}
        )
    endif ()
endfunction()

function (install_dependency arg)
    foreach (package ${ARGV})
        message(STATUS "processing dependency: ${package} (${VCPKG_TARGET_TRIPLET})...")
        execute_process(
            COMMAND ${VCPKG_EXECUTABLE} install ${package} --triplet ${VCPKG_TARGET_TRIPLET} 
            WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
            RESULT_VARIABLE VCPKG_INSTALL_RES
            OUTPUT_QUIET
        )
        if(NOT (${VCPKG_INSTALL_RES} EQUAL 0))
            message(SEND_ERROR "errors occurred when processing dependency ${package} (${VCPKG_TARGET_TRIPLET}).")
        else()
            message(STATUS "succeeded.")
        endif()
    endforeach ()
endfunction()

function (install_vcpkg)
    if(EXISTS "${VCPKG_EXECUTABLE}")
       message(STATUS "use vcpkg executable: ${VCPKG_EXECUTABLE}.")
    else()
        message(STATUS "installing vcpkg...")
        execute_process(
            COMMAND $ENV{ComSpec} /D /E:ON /V:OFF /S /C bootstrap-vcpkg.bat
            WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}\\vcpkg"
            RESULT_VARIABLE VCPKG_INSTALL_RES
            OUTPUT_QUIET
        )
        if(NOT (${VCPKG_INSTALL_RES} EQUAL 0))
            message(SEND_ERROR "errors occurred when installing vcpkg.")
        else()
            message(STATUS "succeeded.")
        endif()
    endif()

endfunction()

macro (find_path_ variable)
    set(options "")
    set(oneValueArgs "")
    set(multiValueArgs NAMES HINTS)
    cmake_parse_arguments(
        ARG 
        "${options}" 
        "${oneValueArgs}"
        "${multiValueArgs}" 
        ${ARGN}
    )
    if ((NOT DEFINED ${variable}) OR (NOT ${variable}))
        find_path (${variable}
            NAMES ${ARG_NAMES}
            HINTS ${ARG_HINTS}
            NO_DEFAULT_PATH)
    endif ()
    if ((NOT DEFINED ${variable}) OR (NOT ${variable}))
        message(SEND_ERROR "cannot find path ${NAMES} with hints ${HINTS}.")
    else ()
        set (${variable})
    endif ()
endmacro ()

macro (find_file_ variable)
    set(options "")
    set(oneValueArgs "")
    set(multiValueArgs NAMES HINTS)
    cmake_parse_arguments(
        ARG 
        "${options}" 
        "${oneValueArgs}"
        "${multiValueArgs}" 
        ${ARGN}
    )
    if ((NOT DEFINED ${variable}) OR (NOT ${variable}))
        find_file (${variable}
            NAMES ${ARG_NAMES}
            HINTS ${ARG_HINTS}
            NO_DEFAULT_PATH)
    endif ()
    if ((NOT DEFINED ${variable}) OR (NOT ${variable}))
        message(SEND_ERROR "cannot find file ${NAMES} with hints ${HINTS}.")
    endif ()
endmacro ()

MACRO(SUBDIRLIST result curdir)
    FILE(GLOB children RELATIVE ${curdir} ${curdir}/*)
    SET(dirlist "")
    FOREACH(child ${children})
        IF(IS_DIRECTORY ${curdir}/${child})
            LIST(APPEND dirlist ${child})
        ENDIF()
    ENDFOREACH()
    SET(${result} ${dirlist})
ENDMACRO()
